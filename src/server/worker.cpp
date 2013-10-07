#include <thread>
#include <chrono>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <vector>
#include <map>

#include "ogrsf_frmts.h"

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
    if (job->getFilter().empty()) {
        poco_information(logger, "pulling layer \""+job->getLayerName()+"\"");
    }
    else {
        poco_information(logger, "pulling layer \""+job->getLayerName()+"\" using filter \""+job->getFilter()+"\"");
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

    // TODO: set filter
    
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
        for(auto tableFieldPair : tableFields) {
            if (tableFieldPair.second.isPrimaryKey) {
                primaryKeyColumns.push_back(tableFieldPair.second.name);
            }
        }
        if (primaryKeyColumns.empty()) {
            throw WorkerError("Got no primarykey for layer \"" + job->getLayerName() + "\"");
        }
        std::vector<std::string> missingPrimaryKeysSource;
        for( auto primaryKeyCol : primaryKeyColumns) {
            if (ogrFields.find(primaryKeyCol) == ogrFields.end()) {
                missingPrimaryKeysSource.push_back(primaryKeyCol);
            }
        }
        if (!missingPrimaryKeysSource.empty()) {
            throw WorkerError("The source for layer \"" + job->getLayerName() + "\" is missing the following fields required "+
                    "by the primary key: " + StringUtils::join(missingPrimaryKeysSource, ", "));
        }
        
        // prepare an insert query into the temporary table 

        // update the existing table

        // insert missing rows in the exisiting table

        // delete deprecated rows from the exisiting table

        job->setStatus(Job::Status::FINISHED);
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
