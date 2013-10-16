#ifndef __batyr_http_notfoundhandler_h__
#define __batyr_http_notfoundhandler_h__


#include "server/http/handler.h"


namespace Batyr 
{
namespace Http
{

    class NotFoundHandler : public Handler
    {
        public:
            NotFoundHandler(Configuration::Ptr _configuration);
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __batyr_http_notfoundhandler_h__
