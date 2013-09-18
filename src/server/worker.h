#ifndef __batyr_worker_h__
#define __batyr_worker_h__

#include <Poco/Logger.h>

#include <memory>

#include "jobstorage.h"


namespace Batyr
{

    class Worker
    {
        private:
            Poco::Logger & logger;
            std::shared_ptr<JobStorage> jobs;

        public:
            Worker(std::shared_ptr<JobStorage> _jobs);

            /** disable copying */
            Worker(const Worker &) = delete;
            Worker& operator=(const Worker &) = delete;

            ~Worker();

            void run();

            typedef std::shared_ptr<Worker> Ptr;

    };

};

#endif // __batyr_worker_h__
