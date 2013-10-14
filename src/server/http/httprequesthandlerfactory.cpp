#include <string>
#include <cstring>
#include <regex>

#include "server/http/httprequesthandlerfactory.h"
#include "server/http/createhandler.h"
#include "server/http/statushandler.h"
#include "server/http/notfoundhandler.h"
#include "server/http/joblisthandler.h"
#include "server/http/getjobhandler.h"
#include "server/http/layerlisthandler.h"
#include "common/config.h"

#ifdef ENABLE_HTTP_WEB_GUI
#include "server/http/bufferhandler.h"
#include "web/http_resources.h"
#endif

using namespace Batyr::Http;


HTTPRequestHandlerFactory::HTTPRequestHandlerFactory(Configuration::Ptr _configuration)
    :   Poco::Net::HTTPRequestHandlerFactory(),
        logger(Poco::Logger::get("Http::HTTPRequestHandlerFactory")),
        configuration(_configuration)
{
}


std::string
HTTPRequestHandlerFactory::normalizeUri(const std::string uri) const
{
    // cut of the request params and the beginning slash
    // to get the name of the endpoint / handler
    std::string endpoint = "";
    size_t pos_end = uri.length();
    if (pos_end > 0) {
        size_t pos_start = uri.find_first_not_of("./");
        if (pos_start == std::string::npos) {
            pos_start = 1;
        }
        size_t params_start = uri.find_first_of("?");
        if (params_start < pos_end){
            pos_end = params_start;
        }
        endpoint = uri.substr(pos_start, pos_end-pos_start);
    }
    return endpoint;
}


Poco::Net::HTTPRequestHandler *
HTTPRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &req)
{
    // cut of the request params and the beginning slash
    // to get the name of the endpoint / handler
    std::string endpoint = normalizeUri(req.getURI());
#ifdef _DEBUG
    std::string logMessage = "Dispatching request " + endpoint;
    poco_debug(logger, logMessage.c_str());
#endif

    // dispatch to api handlers
    if (endpoint == "api/create") {
        auto createHandler = new CreateHandler(configuration);
        createHandler->setJobs(jobs);
        return createHandler;
    }
    else if (endpoint == "api/jobs.json") {
        auto joblistHandler = new JoblistHandler(configuration);
        joblistHandler->setJobs(jobs);
        return joblistHandler;
    }
    else if (endpoint == "api/layers.json") {
        auto layerlistHandler = new LayerlistHandler(configuration);
        return layerlistHandler;
    }
    else if (endpoint == "api/status.json") {
        auto statusHandler = new StatusHandler(configuration);
        statusHandler->setJobs(jobs);
        return statusHandler;
    }

    if (endpoint.compare(0, 8, "api/job/") == 0) {
        size_t posEndId = endpoint.find_first_not_of("abcdef0123456789", 8);
        if ((endpoint.compare(posEndId, endpoint.length() - posEndId, ".json") == 0) && (posEndId != 8)) {
            std::string jobId = endpoint.substr(8, posEndId - 8);

            auto getJobHandler = new GetJobHandler(configuration, jobId);
            getJobHandler->setJobs(jobs);
            return getJobHandler;
        }
    }


#ifdef ENABLE_HTTP_WEB_GUI
    // attempt to satisfy the request with one of the static resources
    // TODO: a std::map would be better than looping over the resources, but as long
    // as there are not to many resources this will also work
    size_t resourceIndex = 0;
    while (resourceIndex < resources_count) {
        struct resource_info * resource = &resources[resourceIndex];

        if ((endpoint == resource->filename) || (endpoint == "" && (strcmp("index.html", resource->filename) == 0))) {
            return new BufferHandler(std::string(resource->mimetype),
                        std::string(resource->etag), resource->data, resource->size_in_bytes);
        }
        resourceIndex++;
    }
#endif

    // at this point everything is just a 404 error
    return new NotFoundHandler;
}
