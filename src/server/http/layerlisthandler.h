#ifndef __batyr_http_layerlisthandler_h__
#define __batyr_http_layerlisthandler_h__


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Logger.h"

#include <memory>

#include "server/jobstorage.h"
#include "server/configuration.h"

namespace Batyr 
{
namespace Http
{

    class LayerlistHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            std::weak_ptr<JobStorage> jobs;
            Poco::Logger & logger;
            Configuration::Ptr configuration;

        public:
            LayerlistHandler(Configuration::Ptr);

            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

            void setJobs(std::weak_ptr<JobStorage> _jobs)
            {
                jobs = _jobs;
            }
            
    };

};
};

#endif // __batyr_http_layerlisthandler_h__
