#include "httplistener.h"

/*
 * http://www.codeproject.com/Articles/252827/Learning-Poco-A-simple-HTTP-server
 */

using namespace Geopoll;


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
HttpListener::start()
{
    poco_debug(logger, "Starting http listener");
}


void
HttpListener::stop()
{
    poco_debug(logger, "Stopping http listener");
}
