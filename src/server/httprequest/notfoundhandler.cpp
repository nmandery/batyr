#include "server/httprequest/notfoundhandler.h"
#include "common/config.h"
#include "common/macros.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

void
NotFoundHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    resp.setContentType("text/plain");
    resp.setReason("Not Found");
    resp.set("Server", APP_NAME_SERVER_FULL);


    std::ostream & out = resp.send();
    out << "Endpoint " << req.getURI() << " not found";
    out.flush();
};
