#ifndef __batyr_httplistener_h__
#define __batyr_httplistener_h__

#include "Poco/Logger.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/SharedPtr.h>

#include <memory>

#include "server/baselistener.h"
#include "server/httprequesthandlerfactory.h"


namespace Batyr {

    class HttpListener : public BaseListener {
        
        private:
            Poco::Logger & logger;
            bool isRunning;

            Poco::Net::ServerSocket socket;
            Poco::Net::HTTPServerParams::Ptr serverParamsPtr;
            Poco::SharedPtr<Batyr::HTTPRequestHandlerFactory> handlerFactoryPtr;
            std::unique_ptr<Poco::Net::HTTPServer> server;


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
