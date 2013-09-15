#include "httplistener.h"
#include "httprequesthandlerfactory.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>


/*
 * http://www.codeproject.com/Articles/252827/Learning-Poco-A-simple-HTTP-server
 */

using namespace Batyr;


HttpListener::HttpListener()
    : BaseListener(), logger(Poco::Logger::get("HttpListener"))
{
    poco_debug(logger, "Setting up http listener");
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
    serverParams->setMaxThreads(5);

    auto handlerFactory = new Batyr::HTTPRequestHandlerFactory; // TODO: check destruction

    Poco::Net::HTTPServer server(handlerFactory, Poco::Net::ServerSocket(9090), serverParams);
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
