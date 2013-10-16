#include "server/http/joblisthandler.h"
#include "server/json.h"
#include "common/macros.h"

#include "rapidjson/document.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::Http;

JobListHandler::JobListHandler(Configuration::Ptr _configuration)
    :   Handler(_configuration),
        logger(Poco::Logger::get("Http::JobListHandler"))
{
}


void
JobListHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    prepareApiResponse(resp);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    // build the json document 
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("maxAgeDoneJobsSeconds", configuration->getMaxAgeDoneJobs(), doc.GetAllocator());

    rapidjson::Value vJobs;
    vJobs.SetArray();
    if (auto jobList = jobs.lock()) {
        auto jobsVec = jobList->getOrderedJobs();

        for(const auto jobP : jobsVec) {
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
