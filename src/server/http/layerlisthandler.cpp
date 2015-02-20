#include "server/http/layerlisthandler.h"
#include "server/json.h"
#include "core/macros.h"

#include "rapidjson/document.h"

#include <Poco/Net/HTTPResponse.h>
#include <iostream>

using namespace Batyr::Http;

LayerListHandler::LayerListHandler(Configuration::Ptr _configuration)
    :   Handler(_configuration),
        logger(Poco::Logger::get("Http::LayerListHandler"))
{
}


void
LayerListHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    prepareApiResponse(resp);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    // build the json document 
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Value vLayers;
    vLayers.SetArray();
    auto layersVec = configuration->getOrderedLayers();
    for(const auto layerP : layersVec) {
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
