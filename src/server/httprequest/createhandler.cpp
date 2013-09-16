#include "createhandler.h"
#include "../job.h"
#include "../../config.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

void
CreateHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    Job job;

    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.setContentType("application/json");
    resp.set("Cache-Control", "no-cache");

    if (req.getMethod() != "POST") {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        resp.setReason("Bad Request");
        job.setErrorMessage("Only POST requests are supported");
    }
    else {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    }

    std::ostream & out = resp.send();
    out << job;
    out.flush();
};
