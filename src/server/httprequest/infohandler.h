#ifndef __batyr_httprequest_infohandler_h__
#define __batyr_httprequest_infohandler_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Logger.h"

#include <memory>

namespace Batyr 
{
namespace HttpRequest 
{

    class InfoHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            Poco::Logger & logger;

        public:
            InfoHandler();
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);

    };

};
};

#endif // __batyr_httprequest_infohandler_h__
