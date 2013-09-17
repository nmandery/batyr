#ifndef __batyr_httprequest_joblisthandler_h__
#define __batyr_httprequest_joblisthandler_h__


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Logger.h"

#include <memory>

#include "../joblist.h"

namespace Batyr 
{
namespace HttpRequest 
{

    class JoblistHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            std::weak_ptr<JobList> jobs;
            Poco::Logger & logger;

        public:
            JoblistHandler();

            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

            void setJobs(std::weak_ptr<JobList> _jobs)
            {
                jobs = _jobs;
            }
            
    };

};
};

#endif // __batyr_httprequest_joblisthandler_h__
