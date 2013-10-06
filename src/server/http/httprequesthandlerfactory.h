#ifndef __batyr_http_httprequesthandlerfactory_h__
#define __batyr_http_httprequesthandlerfactory_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include "Poco/Logger.h"

#include <memory>
#include <string>

#include "server/jobstorage.h"
#include "server/configuration.h"

namespace Batyr
{
namespace Http
{

    class HTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
    {
        public:
            HTTPRequestHandlerFactory(Configuration::Ptr);
            virtual Poco::Net::HTTPRequestHandler * createRequestHandler(const Poco::Net::HTTPServerRequest &);

            void setJobs(std::weak_ptr<JobStorage> _jobs)
            {
                jobs = _jobs;
            }
            
        private:
            Poco::Logger & logger;
            Configuration::Ptr configuration;
            std::weak_ptr<JobStorage> jobs;

            std::string normalizeUri(const std::string) const;
    };
    

};
};

#endif // __batyr_http_httprequesthandlerfactory_h__
