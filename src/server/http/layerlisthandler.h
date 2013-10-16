#ifndef __batyr_http_layerlisthandler_h__
#define __batyr_http_layerlisthandler_h__


#include "Poco/Logger.h"

#include <memory>

#include "server/http/handler.h"

namespace Batyr 
{
namespace Http
{

    class LayerlistHandler : public Handler
    {
        private:
            Poco::Logger & logger;

        public:
            LayerlistHandler(Configuration::Ptr);

            virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp);
    };

};
};

#endif // __batyr_http_layerlisthandler_h__
