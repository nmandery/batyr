#include "createhandler.h"
#include "../config.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

void
CreateHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.setContentType("text/html");
    resp.set("Server", APP_NAME_SERVER_FULL);

    std::ostream & out = resp.send();
    out << "<h1>Hello world!</h1>"
        << "<p>Host: "   << req.getHost()   << "</p>"
        << "<p>Method: " << req.getMethod() << "</p>"
        << "<p>URI: "    << req.getURI()    << "</p>";
    out.flush();
};
