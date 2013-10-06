#include <thread>
#include <chrono>
#include <stdexcept>

#include "common/config.h"
#include "server/worker.h"

using namespace Batyr;


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

            auto layer = configuration->getLayer(job->getLayerName());
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

            // perform the work in an transaction
            if (auto transaction = db.getTransaction()) {

                // build a unique name for the temporary table
                std::string tempTableName = "batyr_" + job->getId();

                // create a temp table to write the data to
                transaction->createTempTable(layer->target_table, tempTableName);

                // fetch the column list from the target_table as the tempTable
                // does not have the constraints of the original table
                auto tableFields = transaction->getTableFields(layer->target_table);

                job->setStatus(Job::Status::FINISHED);
            }
            else {
                std::string msg("Could not start a database transaction");
                poco_error(logger, msg.c_str());
                job->setStatus(Job::Status::FAILED);
                job->setMessage(msg);
            }
        }
        catch (Batyr::Db::DbError &e) {
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
