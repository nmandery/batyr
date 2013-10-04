#ifndef __batyr_http_statushandler_h__
#define __batyr_http_statushandler_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Logger.h"

#include <memory>

#include "server/configuration.h"
#include "server/jobstorage.h"

namespace Batyr 
{
namespace Http
{

    class StatusHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            Poco::Logger & logger;
            Configuration::Ptr configuration;
            std::weak_ptr<JobStorage> jobs;

        public:
            StatusHandler(Configuration::Ptr);
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

            void setJobs(std::weak_ptr<JobStorage> _jobs)
            {
                jobs = _jobs;
            }
 

    };

};
};

#endif // __batyr_http_statushandler_h__
