#ifndef __batyr_broker_h__
#define __batyr_broker_h__

#include <Poco/Logger.h>

#include <vector>
#include <memory>

#include "baselistener.h"
#include "worker.h"
#include "httplistener.h"
#include "jobstorage.h"

namespace Batyr {
   
    class Broker {

        private:
            Poco::Logger & logger;
            std::vector< std::shared_ptr<Batyr::BaseListener> > listeners;
            std::shared_ptr<JobStorage> jobs;
            std::vector< std::shared_ptr< std::thread > > workerThreads;
            std::vector< std::shared_ptr< std::thread > > listenerThreads;


        public:
            Broker();

            /** disable copying */
            Broker(const Broker &) = delete;
            Broker& operator=(const Broker &) = delete;

            ~Broker();

            void addListener( std::shared_ptr<Batyr::BaseListener> );
            void run(size_t _numWorkers);
            void stop();
    };

};

#endif // __batyr_broker_h__
