#ifndef __batyr_httprequesthandlerfactory_h__
#define __batyr_httprequesthandlerfactory_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include "Poco/Logger.h"

#include <memory>

#include "jobstorage.h"
#include "configuration.h"

namespace Batyr {

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
    };
    

};

#endif // __batyr_httprequesthandlerfactory_h__
