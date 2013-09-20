#include "server/httprequest/joblisthandler.h"
#include "server/json.h"
#include "common/config.h"
#include "common/macros.h"

#include "rapidjson/document.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

JoblistHandler::JoblistHandler(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("JoblistHandler")),
        configuration(_configuration)
{
}


void
JoblistHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.set("Cache-Control", "no-cache");
    resp.setContentType("application/json");

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    // build the json document 
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("maxAgeDoneJobsSeconds", configuration->getMaxAgeDoneJobs(), doc.GetAllocator());

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
