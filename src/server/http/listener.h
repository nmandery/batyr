#ifndef __batyr_http_listener_h__
#define __batyr_http_listener_h__

#include "Poco/Logger.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedPtr.h>

#include <memory>

#include "server/baselistener.h"
#include "server/http/httprequesthandlerfactory.h"


namespace Batyr
{
namespace Http
{

    class Listener : public BaseListener {
        
        private:
            Poco::Logger & logger;
            bool isRunning;

            Poco::Net::ServerSocket socket;
            Poco::Net::HTTPServerParams::Ptr serverParamsPtr;
            Poco::SharedPtr<Batyr::Http::HTTPRequestHandlerFactory> handlerFactoryPtr;
            std::unique_ptr<Poco::Net::HTTPServer> server;


        public:

            Listener(Configuration::Ptr);

            /** disable copying */
            Listener(const Listener &) = delete;
            Listener& operator=(const Listener &) = delete;

            ~Listener();


            void run();
            void stop();
    };


};
};


#endif // __batyr_http_listener_h__
