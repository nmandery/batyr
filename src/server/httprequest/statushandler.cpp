#include "statushandler.h"
#include "../../config.h"
#include "../../macros.h"
#include "../json.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;


StatusHandler::StatusHandler()
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("StatusHandler"))
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
    doc.AddMember("numLayers", 0, doc.GetAllocator()); // TODO
    doc.AddMember("numQueuedJobs", 0, doc.GetAllocator()); // TODO
    doc.AddMember("numWorkers", 0, doc.GetAllocator()); // TODO

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(doc);
    out.flush();
};
