#include <string>
#include <cstring>

#include "server/httprequesthandlerfactory.h"
#include "server/httprequest/createhandler.h"
#include "server/httprequest/statushandler.h"
#include "server/httprequest/notfoundhandler.h"
#include "server/httprequest/bufferhandler.h"
#include "server/httprequest/joblisthandler.h"
#include "server/httprequest/layerlisthandler.h"
#include "server/httpassets.h"

using namespace Batyr;


HTTPRequestHandlerFactory::HTTPRequestHandlerFactory(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandlerFactory(),
        logger(Poco::Logger::get("HTTPRequestHandlerFactory")),
        configuration(_configuration)
{
}

Poco::Net::HTTPRequestHandler *
HTTPRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &req)
{
    // cut of the request params and the beginning slash
    // to get the name of the endpoint / handler
    std::string endpoint = "";
    std::string uri = req.getURI();
    size_t pos_start = 0;
    size_t pos_end = uri.length();
    if (pos_end > 0) {
        pos_start = uri.find_first_not_of("./");
        if (pos_start == std::string::npos) {
            pos_start = 1;
        }
        size_t params_start = uri.find_first_of("?");
        if (params_start < pos_end){
            pos_end = params_start;
        }
        endpoint = uri.substr(pos_start, pos_end-pos_start);
    }
#ifdef _DEBUG
    std::string logMessage = "Dispatching request " + endpoint;
    poco_debug(logger, logMessage.c_str());
#endif

    // dispatch to api handlers
    if (endpoint == "api/create") {
        auto createHandler = new Batyr::HttpRequest::CreateHandler;
        createHandler->setJobs(jobs);
        return createHandler;
    }
    else if (endpoint == "api/jobs.json") {
        auto joblistHandler = new Batyr::HttpRequest::JoblistHandler(configuration);
        joblistHandler->setJobs(jobs);
        return joblistHandler;
    }
    else if (endpoint == "api/layers.json") {
        auto layerlistHandler = new Batyr::HttpRequest::LayerlistHandler(configuration);
        return layerlistHandler;
    }
    else if (endpoint == "api/status.json") {
        auto statusHandler = new Batyr::HttpRequest::StatusHandler(configuration);
        return statusHandler;
    }

    // attempt to satisfy the request with one of the static assets
    // TODO: a std::map would be better than looping over the assets, but as long
    // as there are not to many assets this will also work
    size_t assetIndex = 0;
    while (assetIndex < assets_count) {
        struct asset_info * asset = &assets[assetIndex];

        if ((endpoint == asset->filename) || (endpoint == "" && (strcmp("index.html", asset->filename) == 0))) {
            return new Batyr::HttpRequest::BufferHandler(std::string(asset->mimetype),
                        std::string(asset->etag), asset->data, asset->size_in_bytes);
        }
        assetIndex++;
    }

    // at this point everything is just a 404 error
    return new Batyr::HttpRequest::NotFoundHandler;
}
