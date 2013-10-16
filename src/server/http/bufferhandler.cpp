#include "server/http/bufferhandler.h"

using namespace Batyr::Http;

            
BufferHandler::BufferHandler(Configuration::Ptr _configuration, std::string _contentType, std::string _etag, const unsigned char * _buffer, size_t _bufferLen)
    :  Handler(_configuration),
        contentType(_contentType),
        etag(_etag),
        buffer(_buffer),
        bufferLen(_bufferLen)
{
}

void
BufferHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    prepareResponse(resp);
    resp.set("ETag", etag);
    resp.set("Cache-Control", "max-age=300, private");

    if (req.get("If-None-Match", "") == etag) {
        // ETag matched. No content to send;
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_MODIFIED);
        resp.setReason("Not Modified");

        resp.send().flush();
        return;
    }

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.setContentType(contentType);
    resp.setContentLength(bufferLen);

    std::ostream & out = resp.send();
    out.write(reinterpret_cast<const char*>(buffer), bufferLen);
    out.flush();
};
