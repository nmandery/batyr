#ifndef __batyr_http_notfoundhandler_h__
#define __batyr_http_notfoundhandler_h__


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>


namespace Batyr 
{
namespace Http
{

    class NotFoundHandler : public Poco::Net::HTTPRequestHandler
    {
        public:
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __batyr_http_notfoundhandler_h__
