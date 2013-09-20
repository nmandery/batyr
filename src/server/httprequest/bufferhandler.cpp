#include "server/httprequest/bufferhandler.h"

using namespace Batyr::HttpRequest;

            
BufferHandler::BufferHandler(std::string _contentType, std::string _etag, const unsigned char * _buffer, size_t _bufferLen)
    :  Poco::Net::HTTPRequestHandler(),
        contentType(_contentType),
        etag(_etag),
        buffer(_buffer),
        bufferLen(_bufferLen)
{
}

void
BufferHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    resp.set("Server", APP_NAME_SERVER_FULL);
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
