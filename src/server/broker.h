#ifndef __batyr_broker_h__
#define __batyr_broker_h__

#include <Poco/Logger.h>
#include <zmq.hpp>

#include <vector>
#include <memory>

#include "baselistener.h"
#include "httplistener.h"
#include "joblist.h"

namespace Batyr {
   
    class Broker {

        private:
            Poco::Logger & logger;
            std::vector< std::shared_ptr<Batyr::BaseListener> > listeners;
            std::shared_ptr<JobList> jobs;
            
            zmq::context_t * zmq_cx;


        public:
            Broker();
            ~Broker();

            void addListener( std::shared_ptr<Batyr::BaseListener> );
            void run();
            void stop();
    };

};

#endif // __batyr_broker_h__
