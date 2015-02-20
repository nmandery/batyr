#ifndef __batyr_baselistener_h__
#define __batyr_baselistener_h__

#include <Poco/Logger.h>

#include "server/jobstorage.h"
#include "core/configuration.h"

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
             * indicates if the run method of the listener
             * will not imediately return and if the listener
             * shoudl be run in a sperate thread
             */
            virtual bool runInThread() { return false; }

            /**
             * stop the listener.
             * the run method should terminate when this method is called
             */
            virtual void stop() {};

            /** execute the listener. this method may block and should 
             * in that case be executed in a seperate thread
             *
             * Also see the runInThread method
             */
            virtual void run() {};
    };

};


#endif // __batyr_baselistener_h__
