#ifndef __batyr_httprequest_bufferhandler_h__
#define __batyr_httprequest_bufferhandler_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <string>
#include <cstring>

#include "../../config.h"

namespace Batyr
{
namespace HttpRequest 
{

    class BufferHandler : public Poco::Net::HTTPRequestHandler
    {
        private:
            std::string contentType;
            const unsigned char * buffer;
            size_t bufferLen;

        public:
            BufferHandler(std::string _contentType, const unsigned char * _buffer, size_t _bufferLen);
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __batyr_httprequest_bufferhandler_h__
