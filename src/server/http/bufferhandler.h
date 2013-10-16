#ifndef __batyr_http_bufferhandler_h__
#define __batyr_http_bufferhandler_h__

#include <string>
#include <cstring>

#include "server/http/handler.h"

namespace Batyr
{
namespace Http
{

    class BufferHandler : public Handler
    {
        private:
            std::string contentType;
            std::string etag;
            const unsigned char * buffer;
            size_t bufferLen;

        public:
            BufferHandler(Configuration::Ptr _configuration, std::string _contentType, std::string _etag, const unsigned char * _buffer, size_t _bufferLen);
            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __batyr_http_bufferhandler_h__
