#ifndef __batyr_worker_h__
#define __batyr_worker_h__

#include <Poco/Logger.h>

#include <memory>
#include <stdexcept>

#include "server/jobstorage.h"
#include "server/configuration.h"
#include "server/db/connection.h"


namespace Batyr
{

    class WorkerError : public std::runtime_error 
    {
        public:
            WorkerError(const std::string & message) 
                    : std::runtime_error(message)
            {
            };
    };


    class Worker
    {
        private:
            Poco::Logger & logger;
            Configuration::Ptr configuration;
            std::shared_ptr<JobStorage> jobs;
            Batyr::Db::Connection db;

            void pull(Job::Ptr job);

        public:
            Worker(Configuration::Ptr _configuration, std::shared_ptr<JobStorage> _jobs);

            /** disable copying */
            Worker(const Worker &) = delete;
            Worker& operator=(const Worker &) = delete;

            ~Worker();

            void run();

            typedef std::shared_ptr<Worker> Ptr;

    };

};

#endif // __batyr_worker_h__
