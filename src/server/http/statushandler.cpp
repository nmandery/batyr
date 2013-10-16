#include "server/http/statushandler.h"
#include "server/json.h"
#include "common/macros.h"
#include "common/config.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::Http;


StatusHandler::StatusHandler(Configuration::Ptr _configuration)
    :   Handler(_configuration),
        logger(Poco::Logger::get("Http::StatusHandler"))
{
}

void
StatusHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    prepareApiResponse(resp);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("appName", APP_NAME_SERVER, doc.GetAllocator());
    doc.AddMember("appVersion", VERSION_FULL, doc.GetAllocator());
    doc.AddMember("appGitVersion", VERSION_GIT_FULL, doc.GetAllocator());
    doc.AddMember("numLayers", configuration->getLayerCount(), 
                doc.GetAllocator());

    if (auto jobstorage = jobs.lock()) {
        auto jobStats = jobstorage->getStats();

        doc.AddMember("numQueuedJobs", jobStats->numQueuedJobs, doc.GetAllocator());
        doc.AddMember("numFailedJobs", jobStats->numFailedJobs, doc.GetAllocator());
        doc.AddMember("numInProcessJobs", jobStats->numInProcessJobs, doc.GetAllocator());
        doc.AddMember("numFinishedJobs", jobStats->numFinishedJobs, doc.GetAllocator());

    }
    else {
        const char * emsg = "Could not get a lock on jobstorage";
        poco_warning(logger, emsg); // not a severe problem here
    
        doc.AddMember("numQueuedJobs", "?", doc.GetAllocator());
        doc.AddMember("numFailedJobs", "?", doc.GetAllocator());
        doc.AddMember("numInProcessJobs", "?", doc.GetAllocator());
        doc.AddMember("numFinishedJobs", "?", doc.GetAllocator());
    }
    doc.AddMember("numWorkers", configuration->getNumWorkerThreads(),
                doc.GetAllocator());

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(doc);
    out.flush();
};
