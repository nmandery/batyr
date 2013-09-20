#include "server/httprequest/createhandler.h"
#include "server/job.h"
#include "common/config.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;


CreateHandler::CreateHandler()
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("CreateHandler"))
{
}


void
CreateHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    auto job = std::make_shared<Job>();

    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.setContentType("application/json");
    resp.set("Cache-Control", "no-cache");

    if (req.getMethod() != "POST") {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        resp.setReason("Bad Request");
        job->setErrorMessage("Only POST requests are supported");
    }
    else {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    }

    if (auto jobstorage = jobs.lock()) {
        poco_debug(logger, "pushing job to jobstorage");
        jobstorage->push(job);
    }
    else {
        poco_warning(logger, "Could not get a lock on jobstorage");
    }

    std::ostream & out = resp.send();
    out << *job;
    out.flush();
};
