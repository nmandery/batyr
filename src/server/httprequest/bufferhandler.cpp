#include "bufferhandler.h"

#include "../../macros.h"

using namespace Batyr::HttpRequest;

            
BufferHandler::BufferHandler(std::string _contentType, const unsigned char * _buffer, size_t _bufferLen)
    :  Poco::Net::HTTPRequestHandler(),
        contentType(_contentType),
        buffer(_buffer),
        bufferLen(_bufferLen)
{
}

void
BufferHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp)
{
    UNUSED(req)

    resp.set("Server", APP_NAME_SERVER_FULL);
    resp.setContentType(contentType);
    resp.setContentLength(bufferLen);
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

    std::ostream & out = resp.send();
    out.write(reinterpret_cast<const char*>(buffer), bufferLen);
    out.flush();
};
