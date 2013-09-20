#ifndef __batyr_httprequest_statushandler_h__
#define __batyr_httprequest_statushandler_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Logger.h"

#include <memory>

#include "../configuration.h"

namespace Batyr 
{
namespace HttpRequest 
{

    class StatusHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            Poco::Logger & logger;
            Configuration::Ptr configuration;

        public:
            StatusHandler(Configuration::Ptr);
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

    };

};
};

#endif // __batyr_httprequest_statushandler_h__
