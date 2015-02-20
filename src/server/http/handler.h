#ifndef __batyr_http_handler_h__
#define __batyr_http_handler_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <memory>

#include "server/jobstorage.h"
#include "core/configuration.h"


namespace Batyr 
{
namespace Http
{

    class Handler : public Poco::Net::HTTPRequestHandler
    {
        protected:
            Configuration::Ptr configuration;
            std::weak_ptr<JobStorage> jobs;

            void prepareResponse(Poco::Net::HTTPServerResponse &resp);
            void prepareApiResponse(Poco::Net::HTTPServerResponse &resp);

        public:
            Handler(Configuration::Ptr);

            void setJobs(std::weak_ptr<JobStorage> _jobs)
            {
                jobs = _jobs;
            }
            
    };

};
};



#endif // __batyr_http_handler_h__
