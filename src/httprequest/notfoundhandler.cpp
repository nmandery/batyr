#include "notfoundhandler.h"
#include "../config.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

void
NotFoundHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    resp.setContentType("text/plain");
    resp.set("Server", APP_NAME_SERVER_FULL);

    std::ostream & out = resp.send();
    out << "URI "    << req.getURI()    << " not found.";
    out.flush();
};
