#include "layerlisthandler.h"
#include "../json.h"
#include "../../config.h"
#include "../../macros.h"

#include "../../lib/rapidjson/document.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::HttpRequest;

LayerlistHandler::LayerlistHandler(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandler(),
        logger(Poco::Logger::get("LayerlistHandler")),
        configuration(_configuration)
{
}


void
LayerlistHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.set("Cache-Control", "no-cache");
    resp.setContentType("application/json");
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    // build the json document 
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Value vLayers;
    vLayers.SetArray();
    auto layersVec = configuration->getOrderedLayers();
    for(auto layerP : layersVec) {
        rapidjson::Value val;
        val.SetObject();

        rapidjson::Value vName;
        Batyr::Json::toValue(vName, layerP->name, doc.GetAllocator());
        val.AddMember("name", vName, doc.GetAllocator());

        rapidjson::Value vDescription;
        Batyr::Json::toValue(vDescription, layerP->description, doc.GetAllocator());
        val.AddMember("description", vDescription, doc.GetAllocator());

        vLayers.PushBack(val, doc.GetAllocator());
    }
    doc.AddMember("layers", vLayers, doc.GetAllocator());

    std::ostream & out = resp.send();
    out << Batyr::Json::stringify(doc);
    out.flush();
};
