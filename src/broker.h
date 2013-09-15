#ifndef __geopoll_broker_h__
#define __geopoll_broker_h__

#include <Poco/Logger.h>
#include <zmq.hpp>

#include <vector>
#include <memory>

#include "baselistener.h"
#include "httplistener.h"

namespace GeoPoll {
   
    class Broker {

        protected:
            Poco::Logger & logger;
            std::vector< std::shared_ptr<Geopoll::BaseListener> > listeners;
            
            zmq::context_t * zmq_cx;


        public:
            Broker();
            ~Broker();

            void addListener( std::shared_ptr<Geopoll::BaseListener> );
            void run();
            void stop();
    };

};

#endif // __geopoll_broker_h__
