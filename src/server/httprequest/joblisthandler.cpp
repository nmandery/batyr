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
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("maxAgeDoneJobsSeconds", 157, doc.GetAllocator()); // TODO

    rapidjson::Value vJobs;
    vJobs.SetArray();
    if (auto jobList = jobs.lock()) {
        auto jobsVec = jobList->getOrderedJobs();

        for(auto jobP : jobsVec) {
            rapidjson::Value val;
            jobP->toJsonValue(val, doc.GetAllocator());
            vJobs.PushBack(val, doc.GetAllocator());
        }
    }
    else {
        poco_warning(logger, "Could not lock jobList's weak_ptr. So there are no jobs to list available");
    }
    doc.AddMember("jobs", vJobs, doc.GetAllocator());

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(doc);
    out.flush();
};
