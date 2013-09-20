#ifndef __batyr_httplistener_h__
#define __batyr_httplistener_h__

#include "Poco/Logger.h"

#include <mutex>

#include "baselistener.h"

namespace Batyr {

    class HttpListener : public BaseListener {
        
        private:
            Poco::Logger & logger;
            std::mutex runMutex;


        public:

            HttpListener(Configuration::Ptr);

            /** disable copying */
            HttpListener(const HttpListener &) = delete;
            HttpListener& operator=(const HttpListener &) = delete;

            ~HttpListener();


            void run();
            void stop();
    };


};


#endif // __batyr_httplistener_h__
