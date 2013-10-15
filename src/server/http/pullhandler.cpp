#include "server/http/pullhandler.h"
#include "server/job.h"
#include "server/error.h"
#include "common/config.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace Batyr::Http;


PullHandler::PullHandler(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("Http::PullHandler")),
        configuration(_configuration)
{
}


void
PullHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.setContentType("application/json");
    resp.set("Cache-Control", "no-cache");

    if (req.getMethod() != "POST") {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        resp.setReason("Bad Request");

        Error error("Only POST requests are supported");

        std::ostream & out = resp.send();
        out << error;
        out.flush();
        return;
    }


    auto job = std::make_shared<Job>();
    try {
        // read the whole request body into memory
        std::stringstream bodystream;
        bodystream << req.stream().rdbuf();

        // parse received json into the job object
        job->fromString( bodystream.str() );
    }
    catch (std::exception &e) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        resp.setReason("Bad Request");
        
        Error error(e.what());
        poco_warning(logger, e.what());

        std::ostream & out = resp.send();
        out << error;
        out.flush();
        return;
    }

    // return 404 if layer does not exist
    try {
        auto layer = configuration->getLayer(job->getLayerName());
    }
    catch (ConfigurationError) {
        // layer does not exist
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        resp.setReason("Not Found");
        
        std::stringstream msgstream;
        msgstream << "Layer \"" << job->getLayerName() << "\" does not exist";

        Error error(msgstream.str());
        poco_warning(logger, error.getMessage());

        std::ostream & out = resp.send();
        out << error;
        out.flush();
        return;
    }


    if (auto jobstorage = jobs.lock()) {
        poco_debug(logger, "pushing job to jobstorage");
        jobstorage->push(job);
    }
    else {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        resp.setReason("Internal Server Error");
        
        const char * emsg = "Could not get a lock on jobstorage";
        Error error(emsg);
        poco_error(logger, emsg);

        std::ostream & out = resp.send();
        out << error;
        out.flush();
        return;
    }

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    std::ostream & out = resp.send();
    out << *job;
    out.flush();
};
