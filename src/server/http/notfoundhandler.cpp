#include "server/http/notfoundhandler.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::Http;


NotFoundHandler::NotFoundHandler(Configuration::Ptr _configuration)
    :   Handler(_configuration)
{
}

void
NotFoundHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    prepareResponse(resp);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    resp.setContentType("text/plain");
    resp.setReason("Not Found");

    std::ostream & out = resp.send();
    out << "Endpoint " << req.getURI() << " not found";
    out.flush();
};
