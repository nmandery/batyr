#ifndef __geopoll_httplistener_h__
#define __geopoll_httplistener_h__

#include "Poco/Logger.h"

#include "baselistener.h"

namespace Geopoll {

    class HttpListener : public BaseListener {
        
        protected:
            Poco::Logger & logger;

        public:

            HttpListener();
            ~HttpListener();

            void start();
            void stop();
    };


};


#endif // __geopoll_httplistener_h__
