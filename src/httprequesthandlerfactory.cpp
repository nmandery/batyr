#include "httprequesthandlerfactory.h"
#include "httprequest/createhandler.h"
#include "httprequest/notfoundhandler.h"


using namespace Geopoll;


HTTPRequestHandlerFactory::HTTPRequestHandlerFactory()
    :   Poco::Net::HTTPRequestHandlerFactory(),
        logger(Poco::Logger::get("HTTPRequestHandlerFactory"))
{
}

Poco::Net::HTTPRequestHandler *
HTTPRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &req)
{
    if (req.getURI() == "/create") {
        return new Geopoll::HttpRequest::CreateHandler;
    }
    return new Geopoll::HttpRequest::NotFoundHandler;
}
