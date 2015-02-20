#include "server/http/getjobhandler.h"
#include "server/json.h"
#include "server/error.h"
#include "core/macros.h"

#include "rapidjson/document.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>
#include <stdexcept>

using namespace Batyr::Http;

GetJobHandler::GetJobHandler(Configuration::Ptr _configuration, const std::string & _jobId)
    :   Handler(_configuration),
        logger(Poco::Logger::get("Http::GetJobHandler")),
        jobId(_jobId)
{
}


void
GetJobHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    prepareApiResponse(resp);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    // build the json document 
    rapidjson::Document doc;
    bool jobFound = false;

    if (auto jobList = jobs.lock()) {
        try {
            auto job = jobList->getJob(jobId);
            job->toJsonValue(doc,  doc.GetAllocator());
            jobFound = true;
        }
        catch (std::out_of_range) {
            // invalid/non-existing job id
        }
    }
    else {
        poco_warning(logger, "Could not lock jobList's weak_ptr. So there are no jobs to list available");
     }

     if (!jobFound) {
        resp.setReason("Not Found");
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);

        Batyr::Error err("No job with the id " + jobId + " exists.");
        err.toJsonValue(doc,  doc.GetAllocator());
    }

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(doc);
    out.flush();
};
