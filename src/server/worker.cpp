#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <map>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "common/config.h"
#include "common/stringutils.h"
#include "server/worker.h"

using namespace Batyr;

struct OgrField
{
    std::string name;
    unsigned int index;
    OGRFieldType type;
};
typedef std::map<std::string, OgrField> OgrFieldMap;


Worker::Worker(Configuration::Ptr _configuration, std::shared_ptr<JobStorage> _jobs)
    :   logger(Poco::Logger::get("Worker")),
        configuration(_configuration),
        jobs(_jobs),
        db(_configuration)
{
    poco_debug(logger, "Creating Worker");
}


Worker::~Worker()
{
    poco_debug(logger, "Destroying Worker");
}


void
Worker::pull(Job::Ptr job)
{
    auto layer = configuration->getLayer(job->getLayerName());
    bool allow_feature_deletion = layer->allow_feature_deletion;

    // assemble the complete filter to filter the features od the source by
    std::string filterString;
    {
        std::stringstream filterStream;
        if ( (!layer->filter.empty()) && (!job->getFilter().empty()) ) {
            filterStream << "(";
        }
        if (!layer->filter.empty()) {
            filterStream << layer->filter;
        }
        if ( (!layer->filter.empty()) && (!job->getFilter().empty()) ) {
            filterStream << ") and (";
        }
        if (!job->getFilter().empty()) {
            filterStream << job->getFilter();
        }
        if ( (!layer->filter.empty()) && (!job->getFilter().empty()) ) {
            filterStream << ")";
        }
        filterString = filterStream.str();
    }

    // log the job
    {
        std::stringstream initialLogMsgStream;
        initialLogMsgStream     << "job " << job->getId()
                                << " pulling layer \"" << job->getLayerName() << "\"";
        if (!filterString.empty()) {
            initialLogMsgStream << " using filter \"" + filterString + "\"";
        }
        poco_information(logger, initialLogMsgStream.str().c_str());
    }

    // open the dataset
    std::unique_ptr<OGRDataSource, decltype((OGRDataSource::DestroyDataSource))> ogrDataset(
        OGRSFDriverRegistrar::Open(layer->source.c_str(), false),  OGRDataSource::DestroyDataSource);
    if (!ogrDataset) {
        throw WorkerError("Could not open dataset for layer \"" + layer->name + "\"");
    }

    // find the layer
    auto ogrLayer = ogrDataset->GetLayerByName(layer->source_layer.c_str());
    if (ogrLayer == nullptr) {
        throw WorkerError("source_layer \"" +layer->source_layer+ "\" for configured layer \""
                                + layer->name + "\" could not be found");
    }
    ogrLayer->ResetReading();

    // set filter if there is one specified
    if (!filterString.empty()) {

        // when a filter is set the deletion of features is always disabled to allow
        // partial syncs without losing the rest of the data
        allow_feature_deletion = false;

        CPLErrorReset();
        if (ogrLayer->SetAttributeFilter(filterString.c_str()) != OGRERR_NONE) {
            std::stringstream msgstream;
            msgstream   << "The given filter for layer \""
                        << layer->name
                        << "\" is invalid";
            if (CPLGetLastErrorMsg()) {
                msgstream   << ": " << CPLGetLastErrorMsg();
            }
            else {
                msgstream   << ".";
            }
            msgstream   << " The applied filter was [ "
                        << filterString
                        << " ]";
            CPLErrorReset();
            throw WorkerError(msgstream.str());
        }
    }

    auto ogrFeatureDefn = ogrLayer->GetLayerDefn();

#if GDAL_VERSION_MAJOR > 1
    if (ogrFeatureDefn->GetGeomFieldCount() != 1) {
        std::string msg = "The source provides " + std::to_string(ogrFeatureDefn->GetGeomFieldCount()) +
                "geometry fields. Currently only sources with on geoemtry field are supported";
        throw WorkerError(msg);
    }
#endif

    // collect the columns of the dataset
    OgrFieldMap ogrFields;
    for(int i=0; i<ogrFeatureDefn->GetFieldCount(); i++) {
        auto ogrFieldDefn = ogrFeatureDefn->GetFieldDefn(i);

        // lowercase column names -- TODO: this may cause problems when postgresqls column names
        // contain uppercase letters, but will do for a start
        std::string fieldName = StringUtils::tolower(std::string(ogrFieldDefn->GetNameRef()));

#ifdef _DEBUG
        {
            std::string msg = "ogr layer provides the column " + fieldName;
            poco_debug(logger, msg.c_str());
        }
#endif
        auto entry = &ogrFields[fieldName];
        entry->index = i;
        entry->type = ogrFieldDefn->GetType();
    }

    // perform the work in an transaction
    if (auto transaction = db.getTransaction()) {
        int numPulled = 0;
        int numCreated = 0;
        int numUpdated = 0;
        int numDeleted = 0;
        int numIgnored = 0;

        // set the postgresql date style
        transaction->exec("set DateStyle to SQL, YMD");

        // build a unique name for the temporary table
        std::string tempTableName = "batyr_" + job->getId();

        auto versionPostgis = transaction->getPostGISVersion();

        // create a temp table to write the data to
        transaction->createTempTable(layer->target_table_schema, layer->target_table_name, tempTableName);

        // fetch the column list from the target_table as the tempTable
        // does not have the constraints of the original table
        auto tableFields = transaction->getTableFields(layer->target_table_schema, layer->target_table_name);

        // check if the requirements of the primary key are satisfied
        std::vector<std::string> primaryKeyColumns;
        std::string geometryColumn;
        std::vector<std::string> insertColumns;
        std::vector<std::string> updateColumns;
        for(const auto &tableFieldPair : tableFields) {
            if (tableFieldPair.second.isPrimaryKey) {
                primaryKeyColumns.push_back(tableFieldPair.second.name);
            }
            else {
                updateColumns.push_back(tableFieldPair.second.name);
            }
            if (tableFieldPair.second.pgTypeName == "geometry") {
                if (!geometryColumn.empty()) {
                    throw WorkerError("Layer \"" + job->getLayerName() + "\" has multiple geometry columns. Currently only one is supported");
                }
                geometryColumn = tableFieldPair.second.name;
                insertColumns.push_back(tableFieldPair.second.name);
            }
            if (ogrFields.find(tableFieldPair.second.name) != ogrFields.end()) {
                insertColumns.push_back(tableFieldPair.second.name);
            }
        }
        // allow overriding the primarykey from the configfile if there are alternatives cofigured there
        if (!layer->primary_key_columns.empty()) {
            for(const auto primary_key_column : layer->primary_key_columns) {
                if (tableFields.find(primary_key_column) == tableFields.end()) {
                    throw WorkerError("The configured primary key column \"" + primary_key_column + "\" does not exist in the table"
                            " of layer \"" + job->getLayerName() + "\"");
                }
            }
            primaryKeyColumns = layer->primary_key_columns;
        }
        if (primaryKeyColumns.empty()) {
            throw WorkerError("Got no primarykey for layer \"" + job->getLayerName() + "\"");
        }
        std::vector<std::string> missingPrimaryKeysSource;
        for( const auto &primaryKeyCol : primaryKeyColumns) {
            if (ogrFields.find(primaryKeyCol) == ogrFields.end()) {
                missingPrimaryKeysSource.push_back(primaryKeyCol);
            }
        }
        if (!missingPrimaryKeysSource.empty()) {
            throw WorkerError("The source for layer \"" + job->getLayerName() + "\" is missing the following fields required "+
                    "by the primary key: " + StringUtils::join(missingPrimaryKeysSource, ", "));
        }

        // fetch the srid used for the column in postgis
        int pgSrid = 0;
        if (!geometryColumn.empty()) {
            pgSrid = transaction->getGeometryColumnSRID(layer->target_table_schema,
                        layer->target_table_name, geometryColumn);
            poco_debug(logger, "table " + layer->target_table_schema + "." + layer->target_table_name +
                        " column " + geometryColumn + " uses SRID " + std::to_string(pgSrid));
        }

        // prepare an insert query into the temporary table
        std::vector<std::string> insertQueryValues;
        unsigned int idxColumn = 1;
        for (const std::string &insertColumn : insertColumns) {
            auto tableField = &tableFields[insertColumn];
            std::stringstream colStream;

            if (tableField->pgTypeName != "geometry") {
                auto ogrField = &ogrFields[insertColumn];

                colStream   << "$" << idxColumn
                            << "::" << getPostgresType(ogrField->type)
                            << "::" << tableField->pgTypeName;
            }
            else {
                std::stringstream logStream;
                logStream << "job " << job->getId() << " geometry column " << insertColumn;

                if (pgSrid == -1) {
                    logStream   << " uses an undefined SRID (" << pgSrid
                                << "). Can't do any reprojection here, so just assigning the SRID to the new geometries";

                    colStream   << "st_setsrid($" << idxColumn << "::" << tableField->pgTypeName << ", "
                                << pgSrid << ")";
                }
                else if (pgSrid == 0) {
                    logStream   << " has no SRID information. "
                                << "Using the SRID of the geometries as they are read from the source";

                    colStream   <<  "$" << idxColumn << "::" << tableField->pgTypeName;
                }
                else {
                    logStream   << " uses a known, defined SRID (" << pgSrid << "). "
                                << "Reprojecting the geometries if they got a SRS assigned, otherwise assigning the SRID of the table.";

                    // in case the geometries do not have a SRS, assign the one of the table to them
                    colStream   << "(select "
                                <<      "case when st_srid(foo.g) = -1 then "
                                <<          " st_setsrid(foo.g, " << pgSrid << ") "
                                <<      "else "
                                <<          " st_transform(foo.g, " << pgSrid << ") "
                                <<      "end"
                                << " from ( select $" << idxColumn << "::" << tableField->pgTypeName << " as g "
                                << " ) foo)";
                }
                poco_information(logger, logStream.str().c_str());
            }

            insertQueryValues.push_back(colStream.str());
            idxColumn++;
        }
        std::stringstream insertQueryStream;
        insertQueryStream   << "insert into " << transaction->quoteIdent(tempTableName) << " ("
                            << StringUtils::join(transaction->quoteIdent(insertColumns), ", ")
                            << ") "
                            << "select "
                            << StringUtils::join(insertQueryValues, ", ");
        poco_debug(logger, insertQueryStream.str().c_str());
        std::string insertStmtName = "batyr_insert" + job->getId();
        auto resInsertStmt = transaction->prepare(insertStmtName, insertQueryStream.str(), insertColumns.size(), NULL);

        OGRFeature * ogrFeatureP = 0;
        // ensure that features get free'd by wraping them in a smart pointer
        std::unique_ptr<OGRFeature, decltype((OGRFeature::DestroyFeature))> ogrFeature(
                NULL ,  OGRFeature::DestroyFeature);

        while( (ogrFeatureP = ogrLayer->GetNextFeature()) != nullptr) {
            ogrFeature.reset(ogrFeatureP);

            std::vector<PgFieldValue> pgValues;

            for (const std::string &insertColumn : insertColumns) {
                auto tableField = &tableFields[insertColumn];

                if (tableField->pgTypeName == "geometry") {

                    PgFieldValue pgVal;
                    auto ogrGeometry = ogrFeature->GetGeometryRef();
                    if (ogrGeometry != nullptr) {
                        // TODO: Maybe use the implementation from OGRPGLayer::GeometryToHex
                        GByte * buffer;
                        int bufferSize = ogrGeometry->WkbSize();

                        buffer = (GByte *) CPLMalloc(bufferSize);
                        if (buffer == nullptr) {
                            throw WorkerError("Unable to allocate memory to export geometry");
                        }
                        if (ogrGeometry->exportToWkb(wkbNDR, buffer) != OGRERR_NONE) {
                            OGRFree(buffer);
                            throw WorkerError("Could not export the geometry from feature #" + std::to_string(numPulled));
                        }
                        char * hexBuffer = CPLBinaryToHex(bufferSize, buffer);
                        if (hexBuffer == nullptr) {
                            OGRFree(buffer);
                            throw WorkerError("Unable to allocate memory to convert geometry to hex");
                        }
                        OGRFree(buffer);
                        pgVal.setIsNull(bufferSize == 0);
                        if (!pgVal.isNull()) {
                            pgVal.set(std::string(hexBuffer));
                        }
                        CPLFree(hexBuffer);
                    }
                    else {
                        pgVal.setIsNull(true);
                    }
                    pgValues.push_back(std::move(pgVal));
                }
                else {
                    auto ogrField = &ogrFields[insertColumn];
                    PgFieldValue pV = convertToString(ogrFeature.get(), ogrField->index, ogrField->type, tableField->pgTypeName);
                    pgValues.push_back( std::move(pV) );
                }
            }


            // convert to an array of c strings
            std::vector<const char*> cStrValues;
            std::vector<int> cStrValueLenghts;
            std::transform(pgValues.begin(), pgValues.end(), std::back_inserter(cStrValues),
                        [](const PgFieldValue & pV) -> const char * {
                            if (pV.isNull()) {
                                return NULL;
                            }
                            return pV.get().c_str();
                    });
            std::transform(pgValues.begin(), pgValues.end(), std::back_inserter(cStrValueLenghts),
                        [](const PgFieldValue & pV) -> int {
                            if (pV.isNull()) {
                                return 0;
                            }
                            return pV.get().length();
                    });

            if (layer->ignore_failures) {
                try {
                    transaction->exec("savepoint insertfeature;");
                    transaction->execPrepared(insertStmtName, cStrValues.size(), &cStrValues[0],
                                &cStrValueLenghts[0], NULL, 1);
                    transaction->exec("release savepoint insertfeature;");
                }
                catch (Batyr::Db::DbError &e) {
                    if (!e.isDataException()) {
                        throw;
                    }
                    numIgnored++;
                    transaction->exec("rollback to savepoint insertfeature;");

                    std::stringstream ignoreMsgStream;
                    ignoreMsgStream << "Ignoring feature: " << e.what();
                    poco_warning(logger, ignoreMsgStream.str().c_str());
                }
            }
            else {
                transaction->execPrepared(insertStmtName, cStrValues.size(), &cStrValues[0],
                            &cStrValueLenghts[0], NULL, 1);
            }
            numPulled++;
        }
        job->setStatistics(numPulled, numCreated, numUpdated, numDeleted, numIgnored);

        // update the existing table only touching rows which have differences to prevent
        // slowdowns by triggers
        std::stringstream updateStmt;
        updateStmt          << "update " << transaction->quoteIdentJ(layer->target_table_schema, layer->target_table_name) << " "
                            << " set ";
        for (size_t i=0; i<updateColumns.size(); i++) {
            if (i != 0) {
                updateStmt << ", ";
            }
            updateStmt  << transaction->quoteIdent(updateColumns[i]) << " = " 
                        << transaction->quoteIdentJ(tempTableName, updateColumns[i]) << " ";
        }
        updateStmt          << " from " << transaction->quoteIdent(tempTableName) 
                            << " where (";
        for (size_t i=0; i<primaryKeyColumns.size(); i++) {
            if (i != 0) {
                updateStmt << " and ";
            }
            updateStmt  << transaction->quoteIdentJ(layer->target_table_name, primaryKeyColumns[i])
                        << " is not distinct from "
                        << transaction->quoteIdentJ(tempTableName, primaryKeyColumns[i]);
        }
        updateStmt          << ") and (";
        // update only rows which are actual different
        for (size_t i=0; i<updateColumns.size(); i++) {
            if (i != 0) {
                updateStmt << " or ";
            }

            auto tableField = &tableFields[updateColumns[i]];

            if (tableField->pgTypeName == "geometry") {
                std::string quotedTargetGeom =  transaction->quoteIdentJ(layer->target_table_name, updateColumns[i]);
                std::string quotedTempGeom = transaction->quoteIdentJ(tempTableName, updateColumns[i]);

                // update geometries always if the srid differs or when they are collections
                // MEMO: geometries with different SRIDs can not be compared with the "=" operator
                // MEMO: collections can not be compared with st_equals
                updateStmt  << "("
                            <<      "case when "
                            <<          "(st_srid(" << quotedTargetGeom << ") != st_srid(" << quotedTempGeom << ")) ";

                if (std::get<0>(versionPostgis) >= 2) {
                        // st_iscollection is only supported starting with postgis 2.0
                        updateStmt  << " or st_iscollection(" << quotedTargetGeom << ") "
                                    << " or st_iscollection(" << quotedTempGeom << ") ";
                }
                else {
                        updateStmt  << " or ("
                                    <<   "st_geometrytype(" << quotedTargetGeom << ") = 'ST_GeometryCollection'"
                                    <<   " or st_geometrytype(" << quotedTargetGeom << ") like 'ST_Multi%'"
                                    << ") "
                                    << " or ("
                                    <<   "st_geometrytype(" << quotedTempGeom << ") = 'ST_GeometryCollection'"
                                    <<   " or st_geometrytype(" << quotedTempGeom << ") like 'ST_Multi%'"
                                    << ") ";
                }

                updateStmt  <<  "then "
                            // compare using the binary representation as ST_Equals can not be used in this case
                            <<      quotedTargetGeom << "::bytea " << " is distinct from " << quotedTempGeom << "::bytea "
                            <<  " else "
                            // compare using st_equals
                            <<      "not st_equals("  << quotedTargetGeom << ", " << quotedTempGeom << ")"
                            <<  " end "
                            << ")";
            }
            else {
                updateStmt  << "(" << transaction->quoteIdentJ(layer->target_table_name, updateColumns[i])
                            << " is distinct from "
                            << transaction->quoteIdentJ(tempTableName, updateColumns[i]) << ")";
            }
        }
        updateStmt          << ")";
        auto updateRes = transaction->exec(updateStmt.str());
        numUpdated = std::atoi(PQcmdTuples(updateRes.get()));
        updateRes.reset(NULL); // immediately dispose the result

        // insert missing rows in the exisiting table
        std::stringstream insertMissingStmt;
        insertMissingStmt   << "insert into " << transaction->quoteIdentJ(layer->target_table_schema, layer->target_table_name)
                            << " ( " << StringUtils::join(transaction->quoteIdent(insertColumns), ", ") << ") "
                            << " select " << StringUtils::join(transaction->quoteIdent(insertColumns), ", ") << " "
                            << " from " << transaction->quoteIdent(tempTableName)
                            << " where (" << StringUtils::join(transaction->quoteIdent(primaryKeyColumns), ", ") << ") not in ("
                            << " select " << StringUtils::join(transaction->quoteIdent(primaryKeyColumns), ",") << " "
                            << "       from " << transaction->quoteIdentJ(layer->target_table_schema, layer->target_table_name)
                            << ")";
        auto insertMissingRes = transaction->exec(insertMissingStmt.str());
        numCreated = std::atoi(PQcmdTuples(insertMissingRes.get()));
        insertMissingRes.reset(NULL); // immediately dispose the result

        // delete deprecated rows from the exisiting table
        if (allow_feature_deletion) {
            std::stringstream deleteRemovedStmt;
            auto quotedPrimaryKeyColumnsStr = StringUtils::join(transaction->quoteIdent(primaryKeyColumns), ", ");
            deleteRemovedStmt   << "delete from " << transaction->quoteIdentJ(layer->target_table_schema, layer->target_table_name)
                                << " where (" << quotedPrimaryKeyColumnsStr << ") not in ("
                                << " select " << quotedPrimaryKeyColumnsStr << " "
                                << "       from " << transaction->quoteIdent(tempTableName)
                                << ")";
            auto deleteRemovedRes = transaction->exec(deleteRemovedStmt.str());
            numDeleted = std::atoi(PQcmdTuples(deleteRemovedRes.get()));
            deleteRemovedRes.reset(NULL); // immediately dispose the result
        }
        else {
            poco_information(logger, "job " + job->getId() + " feature deletion disabled");
        }

        job->setStatus(Job::Status::FINISHED);
        job->setStatistics(numPulled, numCreated, numUpdated, numDeleted, numIgnored);

        std::stringstream finalLogMsgStream;
        finalLogMsgStream   << "job " << job->getId() << " finished. Stats: "
                            << "pulled=" << numPulled << ", "
                            << "created=" << numCreated << ", "
                            << "updated=" << numUpdated << ", "
                            << "deleted=" << numDeleted;
        poco_information(logger, finalLogMsgStream.str().c_str());
    }
    else {
        std::string msg("Could not start a database transaction");
        poco_error(logger, msg.c_str());
        job->setStatus(Job::Status::FAILED);
        job->setMessage(msg);
    }

}


void
Worker::removeByAttributes(Job::Ptr job)
{
    {
        std::stringstream initialLogMsgStream;
        initialLogMsgStream     << "job " << job->getId()
                                << ": remove-by-attributes on layer \"" << job->getLayerName() << "\"";
        poco_information(logger, initialLogMsgStream.str().c_str());
    }

    auto layer = configuration->getLayer(job->getLayerName());

    if (!layer->allow_feature_deletion) {
        std::string msg = "Layer \"" + job->getLayerName() + "\" does not allow deletion of features.";
        poco_warning(logger, msg.c_str());
        job->setStatus(Job::Status::FAILED);
        job->setMessage(msg);
        return;
    }



    // perform the work in an transaction
    if (auto transaction = db.getTransaction()) {

        // set the postgresql date style
        transaction->exec("set DateStyle to SQL, YMD");

        // fetch the column list from the target_table as the tempTable
        // does not have the constraints of the original table
        auto tableFields = transaction->getTableFields(layer->target_table_schema, layer->target_table_name);

        std::stringstream deleteStmt;
        deleteStmt  << "delete from " << transaction->quoteIdentJ(layer->target_table_schema, layer->target_table_name)
                    << " where ";

        int numDeleted = 0;
        std::vector<Job::AttributeValue> attrValues;
        for(const auto & attributeSet: job->getAttributeSets()) {
            if (attributeSet.size() > 0) {
                int i = 0;
                if (!attrValues.empty()) {
                    deleteStmt << " or ";
                }
                deleteStmt << " ( ";
                for(const auto & attributePair: attributeSet) {
                    auto tableFieldIt = tableFields.find(attributePair.first);
                    if (tableFieldIt == tableFields.end()) {
                        throw WorkerError("Layer \"" + job->getLayerName() + "\" has no field \""
                                        + attributePair.first + "\"");
                    }
                    if (i > 0) {
                        deleteStmt << " and ";
                    }
                    attrValues.push_back(attributePair.second);
                    deleteStmt  << transaction->quoteIdentJ(layer->target_table_name, attributePair.first)
                                << " is not distinct from $" << attrValues.size() << "::" << tableFieldIt->second.pgTypeName << ")";
                    ++i;
                }
                deleteStmt << " ) ";
            }
        }

        if (!attrValues.empty()) {
            poco_debug(logger, deleteStmt.str().c_str());

            // convert to an array of c strings
            std::vector<const char*> cStrValues;
            std::vector<int> cStrValueLenghts;
            std::transform(attrValues.begin(), attrValues.end(), std::back_inserter(cStrValues),
                        [](const Job::AttributeValue & aV) -> const char * {
                            if (aV.isNull()) {
                                return NULL;
                            }
                            return aV.get().c_str();
                    });
            std::transform(attrValues.begin(), attrValues.end(), std::back_inserter(cStrValueLenghts),
                        [](const Job::AttributeValue & aV) -> int {
                            if (aV.isNull()) {
                                return 0;
                            }
                            return aV.get().length();
                    });

            auto deleteRes = transaction->execParams(deleteStmt.str(), cStrValues.size(), NULL, &cStrValues[0],
                        &cStrValueLenghts[0], NULL, 1);
            numDeleted = std::atoi(PQcmdTuples(deleteRes.get()));
        }
        else {
            std::stringstream logMsgStream;
            logMsgStream   << "job " << job->getId() << ": no attributes to remove features given. skipping";
            poco_information(logger, logMsgStream.str().c_str());
        }

        job->setStatus(Job::Status::FINISHED);
        job->setStatistics(0, 0, 0, numDeleted);

        std::stringstream finalLogMsgStream;
        finalLogMsgStream   << "job " << job->getId() << " finished. Stats: "
                            << "deleted=" << numDeleted;
        poco_information(logger, finalLogMsgStream.str().c_str());

    }
    else {
        std::string msg("Could not start a database transaction");
        poco_error(logger, msg.c_str());
        job->setStatus(Job::Status::FAILED);
        job->setMessage(msg);
    }
}


void
Worker::run()
{
    while (true) {
        Job::Ptr job;
        try {
            jobs->popNoWait(job);
            if (!job) {
                // shut down the db connection if this behaviour is configured and
                if (!configuration->usePersistentDbConnections()) {
                    poco_debug(logger, "No jobs queued - closing db connection");
                    db.close();
                }

                // wait for a new job to arrive
                // getting still no job means the queue recieved a quit command, so the worker
                // can be shut down
                jobs->popWait(job);
                if (!job) {
                    break;
                }
            }
            poco_debug(logger, "Got job from queue");
            job->setStatus(Job::Status::IN_PROCESS);

            // check if we got a working database connection
            // or block until we got one
            size_t reconnectAttempts = 0;
            while(!db.reconnect(true)) {
                if (reconnectAttempts == 0) {
                    // set job message to inform clients we are waiting here
                    job->setMessage("Waiting to aquire a database connection");
                }
                reconnectAttempts++;
                std::this_thread::sleep_for( std::chrono::milliseconds( SERVER_DB_RECONNECT_WAIT ) );
            }
            job->setMessage("");

            switch(job->getType()) {
                case Job::Type::PULL:
                    pull(job);
                    break;
                case Job::Type::REMOVE_BY_ATTRIBUTES:
                    removeByAttributes(job);
                    break;
            }
        }
        catch (Batyr::Db::DbError &e) {
            poco_error(logger, e.what());
            job->setStatus(Job::Status::FAILED);
            job->setMessage(e.what());
        }
        catch (WorkerError &e) {
            poco_error(logger, e.what());
            job->setStatus(Job::Status::FAILED);
            job->setMessage(e.what());
        }
        catch (std::runtime_error &e) {
            poco_error(logger, e.what());
            job->setStatus(Job::Status::FAILED);
            job->setMessage(e.what());

            // do not know how this exception was caused as it
            // was not handled by one of the earlier catch blocks
            throw;
        }
    }
    poco_debug(logger, "leaving run method");
}


std::string
Worker::getPostgresType(OGRFieldType fieldType)
{
    std::string pgFieldType;

    switch (fieldType) {
        case OFTString:
            pgFieldType = "text";
            break;
        case OFTInteger:
            pgFieldType = "integer";
            break;
        case OFTReal:
            pgFieldType = "double precision";
            break;
        case OFTDate:
            pgFieldType = "date";
            break;
        case OFTTime:
            pgFieldType = "time";
            break;
        case OFTDateTime:
            pgFieldType = "timestamp";
            break;
        case OFTIntegerList:
            pgFieldType = "integer[]";
            break;
        case OFTRealList:
            pgFieldType = "double precision[]";
            break;
        case OFTStringList:
            pgFieldType = "text[]";
            break;
        case OFTBinary:
            pgFieldType = "bytea";
            break;
        default:
            throw WorkerError("Unsupported OGR field type: " + std::to_string(static_cast<int>(fieldType)));
    }
    return pgFieldType;
}


PgFieldValue
Worker::convertToString(OGRFeature * ogrFeature, const int fieldIdx, OGRFieldType fieldType, const std::string pgTypeName)
{
    PgFieldValue result;

    switch (fieldType) {
        case OFTString:
            {
                const char * fieldValue = ogrFeature->GetFieldAsString(fieldIdx);
                result.setIsNull((fieldValue == nullptr));
                if (!result.isNull()) {
                    result.set(fieldValue);
                }
                break;
            }
        case OFTInteger:
            result.setIsNull(false);
            result.set(std::to_string(ogrFeature->GetFieldAsInteger(fieldIdx)));
            break;
        case OFTReal:
            result.setIsNull(false);
            result.set(std::to_string(ogrFeature->GetFieldAsDouble(fieldIdx)));
            break;
        case OFTDate:
        case OFTTime:
        case OFTDateTime:
            {
                int dtYear = 0;
                int dtMonth = 0;
                int dtDay = 0;
                int dtHour = 0;
                int dtMinute = 0;
                int dtSecond = 0;
                int dtTZFlag = 0;

                if (ogrFeature->GetFieldAsDateTime(fieldIdx, &dtYear, &dtMonth, &dtDay,
                            &dtHour, &dtMinute, &dtSecond, &dtTZFlag)) {
                    char buf[128] = {0,};

                    if (fieldType == OFTDate) {
                        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", dtYear, dtMonth, dtDay);
                    }
                    else if (fieldType == OFTTime) {
                        std::snprintf(buf, sizeof(buf), "%02d:%02d:%09d.0", dtHour, dtMinute, dtSecond);
                    }
                    else if (fieldType == OFTDateTime) {
                        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%09d.0",
                                 dtYear, dtMonth, dtDay, dtHour, dtMinute, dtSecond);
                    }
                    else {
                        throw WorkerError("None of the implemented date/time/datetime types matched. This point should not be reacheable");
                    }
                    result.setIsNull(false);
                    result.set(std::string(buf));
                }
                else {
                    result.setIsNull(true);
                    poco_debug(logger, "field at index " + std::to_string(fieldIdx) + " is null");
                }
                break;
            }
        // TODO: implement all of the OGRFieldType types
        //  case OFTIntegerList:
        //  case OFTRealList:
        //  case OFTStringList:
        //  case OFTBinary:
        default:
            throw WorkerError("Unsupported OGR field type: " + std::to_string(static_cast<int>(fieldType)));
    }

    // sanitize invalid values - empty time types are generaly invalid and most certainly result from
    // string fields returned as empty strings
    if (result.get().empty() && (
                (pgTypeName == "timestamp") ||
                (pgTypeName == "timestampz") ||
                (pgTypeName == "time") ||
                (pgTypeName == "date")
            )) {
        result.setIsNull(true);
    }

    return std::move(result);
}
