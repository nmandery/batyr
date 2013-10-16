#include "server/http/handler.h"
#include "common/config.h"

using namespace Batyr::Http;


Handler::Handler(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandler(),
        configuration(_configuration)
{
}


void 
Handler::prepareResponse(Poco::Net::HTTPServerResponse &resp)
{
    resp.set("Server", APP_NAME_SERVER_FULL);
}

void
Handler::prepareApiResponse(Poco::Net::HTTPServerResponse &resp)
{
    prepareResponse(resp);
    resp.setContentType("application/json");
    resp.set("Cache-Control", "no-cache");
}
