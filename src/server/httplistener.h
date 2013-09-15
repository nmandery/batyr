#ifndef __batyr_httplistener_h__
#define __batyr_httplistener_h__

#include "Poco/Logger.h"

#include <mutex>

#include "baselistener.h"

namespace Batyr {

    class HttpListener : public BaseListener {
        
        protected:
            Poco::Logger & logger;

            std::mutex runMutex;

        public:

            HttpListener();
            ~HttpListener();

            void run();
            void stop();
    };


};


#endif // __batyr_httplistener_h__
