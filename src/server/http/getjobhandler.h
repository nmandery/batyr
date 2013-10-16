#ifndef __batyr_http_getjobhandler_h__
#define __batyr_http_getjobhandler_h__

#include "Poco/Logger.h"

#include <memory>

#include "server/http/handler.h"

namespace Batyr 
{
namespace Http
{

    class GetJobHandler : public Handler
    {
        private:
            Poco::Logger & logger;
            std::string jobId;

        public:
            GetJobHandler(Configuration::Ptr, const std::string &);

            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

    };

};
};

#endif // __batyr_http_getjobhandler_h__
