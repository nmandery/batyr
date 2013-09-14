#ifndef __geopoll_httprequesthandlerfactory_h__
#define __geopoll_httprequesthandlerfactory_h__

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include "Poco/Logger.h"

namespace Geopoll {

    class HTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
    {
        public:
            HTTPRequestHandlerFactory();
            virtual Poco::Net::HTTPRequestHandler * createRequestHandler(const Poco::Net::HTTPServerRequest &);

        protected:
            Poco::Logger & logger;
    };
    

};

#endif // __geopoll_httprequesthandlerfactory_h__
