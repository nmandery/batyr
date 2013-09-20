#ifndef __batyr_baselistener_h__
#define __batyr_baselistener_h__

#include <Poco/Logger.h>

#include "jobstorage.h"
#include "configuration.h"

#include <memory>

namespace Batyr {

    class BaseListener {
        
        private:
            Poco::Logger & logger;

        protected:
            std::weak_ptr<JobStorage> jobs;
            Configuration::Ptr configuration;

        public:

            BaseListener(Configuration::Ptr);

            /** disable copying */
            BaseListener(const BaseListener &) = delete;
            BaseListener& operator=(const BaseListener &) = delete;

            ~BaseListener();

            void setJobs(std::shared_ptr<JobStorage> _jobs)
            {
                jobs = _jobs;
            }

            /**
             * stop the listener.
             * the run method should terminate when this method is called
             */
            virtual void stop() {};

            /** execute the listener. this method will block and should be
             * executed in a seperate thread
             */
            virtual void run() {};
    };

};


#endif // __batyr_baselistener_h__
