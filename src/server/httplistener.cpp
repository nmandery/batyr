#include "httplistener.h"
#include "httprequesthandlerfactory.h"
#include "../config.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>


/*
 * http://www.codeproject.com/Articles/252827/Learning-Poco-A-simple-HTTP-server
 */

using namespace Batyr;


HttpListener::HttpListener(Configuration::Ptr _configuration)
    :   BaseListener(_configuration), 
        logger(Poco::Logger::get("HttpListener"))
{
}

HttpListener::~HttpListener() 
{
    stop();
}

void
HttpListener::run()
{
    poco_debug(logger, "Starting http listener");
    runMutex.lock(); // TODO: use try_lock and log failures

    auto serverParams = new Poco::Net::HTTPServerParams; // TODO: check destruction
    serverParams->setMaxThreads( SERVER_HTTP_THREADS );

    auto handlerFactory = new Batyr::HTTPRequestHandlerFactory(configuration); // TODO: check destruction
    handlerFactory->setJobs(jobs);

    Poco::Net::HTTPServer server(handlerFactory, 
            Poco::Net::ServerSocket( configuration->getHttpPort() ), 
            serverParams);
    server.start();

    // wait until shutdown
    runMutex.lock();

    server.stop();
    runMutex.unlock();
}


void
HttpListener::stop()
{
    poco_debug(logger, "Stopping http listener");
    runMutex.unlock();
}
