#include "server/http/statushandler.h"
#include "common/config.h"
#include "common/macros.h"
#include "server/json.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::Http;


StatusHandler::StatusHandler(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("Http::StatusHandler")),
        configuration(_configuration)
{
}

void
StatusHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.setContentType("application/json");
    resp.set("Cache-Control", "no-cache");
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("appName", APP_NAME_SERVER, doc.GetAllocator());
    doc.AddMember("appVersion", VERSION_FULL, doc.GetAllocator());
    doc.AddMember("numLayers", configuration->getLayerCount(), 
                doc.GetAllocator());

    if (auto jobstorage = jobs.lock()) {
        doc.AddMember("numQueuedJobs", jobstorage->queueSize(), doc.GetAllocator());
    }
    else {
        const char * emsg = "Could not get a lock on jobstorage";
        poco_warning(logger, emsg); // not a severe problem here
    
        doc.AddMember("numQueuedJobs", "?", doc.GetAllocator());
    }
    doc.AddMember("numWorkers", configuration->getNumWorkerThreads(),
                doc.GetAllocator());

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(doc);
    out.flush();
};
