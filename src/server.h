#ifndef __geopoll_server_h__
#define __geopoll_server_h__

#include <Poco/Util/ServerApplication.h>

#include <string>
#include <vector>

namespace Geopoll {

    class Server : public Poco::Util::ServerApplication
    {
        protected:
            int main(const std::vector<std::string> &);

            /** initialize the logging system */
            void initLogging();
    };

};

#endif // __geopoll_server_h__
