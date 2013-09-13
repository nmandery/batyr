#ifndef __geopoll_baselistener_h__
#define __geopoll_baselistener_h__

#include "Poco/Logger.h"

#include <memory>

namespace Geopoll {

    class BaseListener {
        
        protected:
            Poco::Logger & logger;

        public:

            BaseListener();
            ~BaseListener();

            virtual void start() {};
            virtual void stop() {};
    };

};


#endif // __geopoll_baselistener_h__
