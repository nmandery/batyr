#ifndef __batyr_baselistener_h__
#define __batyr_baselistener_h__

#include <Poco/Logger.h>

#include <memory>

namespace Batyr {

    class BaseListener {
        
        protected:
            Poco::Logger & logger;

        public:

            BaseListener();
            ~BaseListener();

            virtual void stop() {};

            /** execute the listener. this method will block and should be
             * executed in a seperate thread
             */
            virtual void run() {};
    };

};


#endif // __batyr_baselistener_h__
