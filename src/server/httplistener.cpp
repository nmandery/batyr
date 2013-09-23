#include "server/httplistener.h"
#include "common/config.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Exception.h>


/*
 * http://www.codeproject.com/Articles/252827/Learning-Poco-A-simple-HTTP-server
 */

using namespace Batyr;


HttpListener::HttpListener(Configuration::Ptr _configuration)
    :   BaseListener(_configuration), 
        logger(Poco::Logger::get("HttpListener")),
        isRunning(false)
{
    // prepare the server parameters and let them be managed by
    // a smart pointer
    auto serverParams = new Poco::Net::HTTPServerParams;
    serverParams->setMaxThreads( SERVER_HTTP_THREADS );
    serverParamsPtr.assign(serverParams);
    
    // set up the network socket
    try {
        socket.bind( configuration->getHttpPort() );
        socket.listen();
    }
    catch (Poco::Exception& e) {
        poco_error(logger, e.message());
        e.rethrow();
    }

    // will get destroyed with by the poco httpserver
    auto handlerFactory = new Batyr::HTTPRequestHandlerFactory(configuration);
    handlerFactoryPtr.assign(handlerFactory);

    server.reset(new Poco::Net::HTTPServer(handlerFactoryPtr, socket, serverParamsPtr));
}

HttpListener::~HttpListener() 
{
    stop();
    socket.close();
}

void
HttpListener::run()
{
    poco_debug(logger, "Starting http listener");
    if (!isRunning) {

        // set the joblist
        handlerFactoryPtr->setJobs(jobs);

        server->start();
        isRunning = true;
    }
}


void
HttpListener::stop()
{
    poco_debug(logger, "Stopping http listener");
    if (isRunning) {
        server->stop();
        isRunning = false;
    }
}
