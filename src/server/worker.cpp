#include <algorithm>
#include <cctype>
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
    {
        std::stringstream initialLogMsgStream;
        initialLogMsgStream     << "job " << job->getId() 
                                << ": pulling layer \"" << job->getLayerName() << "\"";
        if (!job->getFilter().empty()) {
            initialLogMsgStream << " using filter \""+job->getFilter()+"\"";
        }
        poco_information(logger, initialLogMsgStream.str().c_str());
    }

    auto layer = configuration->getLayer(job->getLayerName());

    // open the dataset
    std::unique_ptr<OGRDataSource, void (*)(OGRDataSource*)> ogrDataset(
        OGRSFDriverRegistrar::Open(layer->source.c_str(), false),  OGRDataSource::DestroyDataSource);
    if (!ogrDataset) {
        throw WorkerError("Could not open dataset for layer \"" + layer->name + "\"");
    }

    // find the layer
    auto ogrLayer = ogrDataset->GetLayerByName(layer->source_layer.c_str());
    if (ogrLayer == nullptr) {
        throw WorkerError("source_layer \"" +layer->source_layer+ "\" in dataset for layer \""
                                + layer->name + "\" not found");
    }
    ogrLayer->ResetReading();

    // set filter if set
    std::string filterString = job->getFilter();
    if (!filterString.empty()) {
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
        std::string fieldNameCased = std::string(ogrFieldDefn->GetNameRef());
        std::string fieldName;
        std::transform(fieldNameCased.begin(), fieldNameCased.end(), std::back_inserter(fieldName), ::tolower);

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

        // set the postgresql date style
        transaction->exec("set DateStyle to SQL, YMD");

        // build a unique name for the temporary table
        std::string tempTableName = "batyr_" + job->getId();

        // create a temp table to write the data to
        transaction->createTempTable(layer->target_table_schema, layer->target_table_name, tempTableName);

        // fetch the column list from the target_table as the tempTable
        // does not have the constraints of the original table
        auto tableFields = transaction->getTableFields(layer->target_table_schema, layer->target_table_name);

        // check if the requirements of the primary key are satisfied
        // TODO: allow overriding the primarykey from the configfile
        std::vector<std::string> primaryKeyColumns;
        std::string geometryColumn;
        std::vector<std::string> insertColumns;
        std::vector<std::string> updateColumns;
        for(auto &tableFieldPair : tableFields) {
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
        if (primaryKeyColumns.empty()) {
            throw WorkerError("Got no primarykey for layer \"" + job->getLayerName() + "\"");
        }
        std::vector<std::string> missingPrimaryKeysSource;
        for( auto &primaryKeyCol : primaryKeyColumns) {
            if (ogrFields.find(primaryKeyCol) == ogrFields.end()) {
                missingPrimaryKeysSource.push_back(primaryKeyCol);
            }
        }
        if (!missingPrimaryKeysSource.empty()) {
            throw WorkerError("The source for layer \"" + job->getLayerName() + "\" is missing the following fields required "+
                    "by the primary key: " + StringUtils::join(missingPrimaryKeysSource, ", "));
        }

        // prepare an insert query into the temporary table
        std::vector<std::string> insertQueryValues;
        unsigned int idxColumn = 1;
        for (std::string &insertColumn : insertColumns) {
            auto tableField = &tableFields[insertColumn];
            std::stringstream colStream;
            colStream   << "$" << idxColumn;
            if (tableField->pgTypeName != "geometry") {
                auto ogrField = &ogrFields[insertColumn];
                colStream << "::" << getPostgresType(ogrField->type);
            }
            colStream   << "::" << tableFields[insertColumn].pgTypeName;
            insertQueryValues.push_back(colStream.str());
            idxColumn++;
        }
        std::stringstream insertQueryStream;
        // TODO: include st_transform statement into insert if original table has a srid set in geometry_columns
        insertQueryStream   << "insert into \"" << tempTableName << "\" (\""
                            << StringUtils::join(insertColumns, "\", \"")
                            << "\") values ("
                            << StringUtils::join(insertQueryValues, ", ")
                            << ")";
        poco_debug(logger, insertQueryStream.str().c_str());
        std::string insertStmtName = "batyr_insert" + job->getId();
        auto resInsertStmt = transaction->prepare(insertStmtName, insertQueryStream.str(), insertColumns.size(), NULL);

        OGRFeature * ogrFeature = 0;
        while( (ogrFeature = ogrLayer->GetNextFeature()) != nullptr) {
            std::vector<PgFieldValue> pgValues;

            for (std::string &insertColumn : insertColumns) {
                auto tableField = &tableFields[insertColumn];

                if (tableField->pgTypeName == "geometry") {
                    // TODO: Maybe use the implementation from OGRPGLayer::GeometryToHex
                    GByte * buffer;

                    auto ogrGeometry = ogrFeature->GetGeometryRef();
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
                    PgFieldValue pgVal;
                    pgVal.first = (bufferSize == 0);
                    pgVal.second = std::string(hexBuffer);
                    pgValues.push_back(std::move(pgVal));
                    CPLFree(hexBuffer);
                }
                else {
                    auto ogrField = &ogrFields[insertColumn];
                    PgFieldValue pV = convertToString(ogrFeature, ogrField->index, ogrField->type, tableField->pgTypeName);
                    pgValues.push_back( std::move(pV) );
                }
            }


            // convert to an array of c strings
            std::vector<const char*> cStrValues;
            std::vector<int> cStrValueLenghts;
            std::transform(pgValues.begin(), pgValues.end(), std::back_inserter(cStrValues),
                        [](PgFieldValue & pV) -> const char * {
                            if (pV.first) {
                                return NULL;
                            }
                            return pV.second.c_str();
                    });
            std::transform(pgValues.begin(), pgValues.end(), std::back_inserter(cStrValueLenghts),
                        [](PgFieldValue & pV) -> int {
                            if (pV.first) {
                                return 0;
                            }
                            return pV.second.length();
                    });

            transaction->execPrepared(insertStmtName, cStrValues.size(), &cStrValues[0], &cStrValueLenghts[0],
                        NULL, 1);

            numPulled++;
        }
        job->setStatistics(numPulled, numCreated, numUpdated, numDeleted);

        // update the existing table only touching rows which have differences to prevent
        // slowdowns by triggers
        std::stringstream updateStmt;
        updateStmt          << "update \"" << layer->target_table_schema << "\".\"" << layer->target_table_name << "\" "
                            << " set ";
        for (size_t i=0; i<updateColumns.size(); i++) {
            if (i != 0) {
                updateStmt << ", ";
            }
            updateStmt << "\"" << updateColumns[i] << "\" = \"" << tempTableName << "\".\"" << updateColumns[i] << "\" ";
        }
        updateStmt          << " from \"" << tempTableName << "\""
                            << " where (";
        for (size_t i=0; i<primaryKeyColumns.size(); i++) {
            if (i != 0) {
                updateStmt << " and ";
            }
            updateStmt  << "\""  << layer->target_table_name << "\".\"" << primaryKeyColumns[i]
                        << "\" is not distinct from \"" << tempTableName << "\".\"" << primaryKeyColumns[i] << "\"";
        }
        updateStmt          << ") and (";
        // update only rows which are actual different
        for (size_t i=0; i<updateColumns.size(); i++) {
            if (i != 0) {
                updateStmt << " or ";
            }
            updateStmt  << "(\"" << layer->target_table_name << "\".\"" << updateColumns[i]
                        << "\" is distinct from  \""
                        << tempTableName << "\".\"" << updateColumns[i] << "\")";
        }
        updateStmt          << ")";
        auto updateRes = transaction->exec(updateStmt.str());
        numUpdated = std::atoi(PQcmdTuples(updateRes.get()));
        updateRes.reset(NULL); // immediately dispose the result

        // insert missing rows in the exisiting table
        std::stringstream insertMissingStmt;
        insertMissingStmt   << "insert into \"" << layer->target_table_schema << "\".\"" << layer->target_table_name << "\" "
                            << " ( \"" << StringUtils::join(insertColumns, "\", \"") << "\") "
                            << " select \"" << StringUtils::join(insertColumns, "\", \"") << "\" "
                            << " from \"" << tempTableName << "\""
                            << " where (\"" << StringUtils::join(primaryKeyColumns, "\", \"") << "\") not in ("
                            << " select \"" << StringUtils::join(primaryKeyColumns, "\",\"") << "\" "
                            << "       from \"" << layer->target_table_schema << "\".\""  << layer->target_table_name << "\""
                            << ")";
        auto insertMissingRes = transaction->exec(insertMissingStmt.str());
        numCreated = std::atoi(PQcmdTuples(insertMissingRes.get()));
        insertMissingRes.reset(NULL); // immediately dispose the result

        // delete deprecated rows from the exisiting table
        // TODO: make this optional and skip when a filter is used
        std::stringstream deleteRemovedStmt;
        deleteRemovedStmt   << "delete from \"" << layer->target_table_schema << "\".\"" << layer->target_table_name << "\" "
                            << " where (\"" << StringUtils::join(primaryKeyColumns, "\", \"") << "\") not in ("
                            << " select \"" << StringUtils::join(primaryKeyColumns, "\",\"") << "\" "
                            << "       from \"" << tempTableName << "\""
                            << ")";
        auto deleteRemovedRes = transaction->exec(deleteRemovedStmt.str());
        numDeleted = std::atoi(PQcmdTuples(deleteRemovedRes.get()));
        deleteRemovedRes.reset(NULL); // immediately dispose the result

        job->setStatus(Job::Status::FINISHED);
        job->setStatistics(numPulled, numCreated, numUpdated, numDeleted);

        std::stringstream finalLogMsgStream;
        finalLogMsgStream   << "job " << job->getId() << " finished. Stats: "
                            << "pulled=" << numPulled << ", "
                            << "created=" << numCreated << ", "
                            << "updated=" << numUpdated << ", "
                            << "deleted=" << numDeleted << "";
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
            bool got_job = jobs->pop(job);
            if (!got_job) {
                // no job means the queue recieved a quit command, so the worker
                // can be shut down
                break;
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

            pull(job);
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
                result.first = (fieldValue == nullptr);
                result.second = fieldValue;
                break;
            }
        case OFTInteger:
            result.first = false;
            result.second = std::to_string(ogrFeature->GetFieldAsInteger(fieldIdx));
            break;
        case OFTReal:
            result.first = false;
            result.second = std::to_string(ogrFeature->GetFieldAsDouble(fieldIdx));
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
                    result.first = false;
                    result.second = std::string(buf);
                }
                else {
                    result.first = true;
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
    if (result.second.empty() && (
                (pgTypeName == "timestamp") ||
                (pgTypeName == "timestampz") ||
                (pgTypeName == "time") ||
                (pgTypeName == "date")
            )) {
        result.first = true;
        result.second.clear();
    }

    return std::move(result);
}
