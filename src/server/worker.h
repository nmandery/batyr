#ifndef __batyr_worker_h__
#define __batyr_worker_h__

#include <Poco/Logger.h>

#include <memory>
#include <utility>
#include <stdexcept>

#include "server/jobstorage.h"
#include "core/configuration.h"
#include "core/db/connection.h"
#include "core/db/queryvalue.h"

#include "ogrsf_frmts.h"


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
            void removeByAttributes(Job::Ptr job);

            /**
             * convert the field at the given index with the given
             * OGRFieldType to a postgresql compatible string
             */
            QueryValue convertToString(OGRFeature * ogrFeature, const int fieldIdx, OGRFieldType fieldType, const std::string pgTypeName);

            std::string getPostgresType(OGRFieldType fieldType);

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
