#ifndef __batyr_http_getjobhandler_h__
#define __batyr_http_getjobhandler_h__


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

    class GetJobHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            std::weak_ptr<JobStorage> jobs;
            Poco::Logger & logger;
            Configuration::Ptr configuration;
            std::string jobId;

        public:
            GetJobHandler(Configuration::Ptr, const std::string &);

            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

            void setJobs(std::weak_ptr<JobStorage> _jobs)
            {
                jobs = _jobs;
            }
            
    };

};
};

#endif // __batyr_http_getjobhandler_h__
