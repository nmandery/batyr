#ifndef __batyr_http_joblisthandler_h__
#define __batyr_http_joblisthandler_h__

#include "Poco/Logger.h"

#include <memory>

#include "server/http/handler.h"

namespace Batyr 
{
namespace Http
{

    class JoblistHandler : public Handler
    {
        private:
            Poco::Logger & logger;

        public:
            JoblistHandler(Configuration::Ptr);
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __batyr_http_joblisthandler_h__
