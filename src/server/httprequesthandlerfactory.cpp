#include <string>
#include <cstring>

#include "httprequesthandlerfactory.h"
#include "httprequest/createhandler.h"
#include "httprequest/notfoundhandler.h"
#include "httprequest/bufferhandler.h"
#include "httpassets.h"

using namespace Batyr;


HTTPRequestHandlerFactory::HTTPRequestHandlerFactory()
    :   Poco::Net::HTTPRequestHandlerFactory(),
        logger(Poco::Logger::get("HTTPRequestHandlerFactory"))
{
}

Poco::Net::HTTPRequestHandler *
HTTPRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &req)
{
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
    poco_debug(logger, "Dispatching request for endpoint:");
    poco_debug(logger, endpoint.c_str());

    // dispatch to api handlers
    if (req.getURI() == "create") {
        return new Batyr::HttpRequest::CreateHandler;
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
