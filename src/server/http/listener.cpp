#include "server/http/listener.h"
#include "common/config.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Exception.h>


using namespace Batyr::Http;


Listener::Listener(Configuration::Ptr _configuration)
    :   Batyr::BaseListener(_configuration), 
        logger(Poco::Logger::get("Http::Listener")),
        isRunning(false)
{
    // prepare the server parameters and let them be managed by
    // a smart pointer
    serverParamsPtr.assign( new Poco::Net::HTTPServerParams );
    serverParamsPtr->setMaxThreads( SERVER_HTTP_THREADS );
    
    // set up the network socket
    try {
        // set reuse flags on the socket to prevent "Adress already in use"
        // errors on quick restarts of the server when the TCP socket
        // is still in TIME_WAIT state
        //socket.setReuseAddress(true);
        socket.setReusePort(true);

        socket.bind( configuration->getHttpPort() );
        socket.listen();
    }
    catch (Poco::Exception& e) {
        poco_error(logger, e.message());
        e.rethrow();
    }

    // will get destroyed with by the poco httpserver
    handlerFactoryPtr.assign( new HTTPRequestHandlerFactory(configuration) );

    server.reset(new Poco::Net::HTTPServer(handlerFactoryPtr, socket, serverParamsPtr));
}

Listener::~Listener() 
{
    stop();
    socket.close();
}

void
Listener::run()
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
Listener::stop()
{
    poco_debug(logger, "Stopping http listener");
    if (isRunning) {
        server->stop();
        isRunning = false;
    }
}
