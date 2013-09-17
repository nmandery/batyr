#include "joblisthandler.h"
#include "../json.h"
#include "../../config.h"

#include "../../lib/rapidjson/document.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

JoblistHandler::JoblistHandler()
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("JoblistHandler"))
{
}


void
JoblistHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.set("Cache-Control", "no-cache");

    if (req.getMethod() != "GET") {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        resp.setContentType("text/plain");
        resp.setReason("Bad Request");
        
        std::ostream & out = resp.send();
        out << "Only GET requests are supported";
        out.flush();
        return;
    }

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.setContentType("application/json");

    // build the json document 
    rapidjson::Document data;
    data.SetArray();

    if (auto jobList = jobs.lock()) {
        auto jobsVec = jobList->getOrderedJobs();

        for(auto jobP : jobsVec) {
            rapidjson::Value val;
            jobP->toJsonValue(val, data.GetAllocator());
            data.PushBack(val, data.GetAllocator());
        }
    }
    else {
        poco_warning(logger, "Could not lock jobList's weak_ptr. So there are no jobs to list available");
    }

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(data);
    out.flush();
};
