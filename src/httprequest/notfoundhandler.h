#ifndef __geopoll_httprequest_notfoundhandler_h__
#define __geopoll_httprequest_notfoundhandler_h__


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>


namespace Geopoll 
{
namespace HttpRequest 
{

    class NotFoundHandler : public Poco::Net::HTTPRequestHandler
    {
        public:
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __geopoll_httprequest_notfoundhandler_h__
