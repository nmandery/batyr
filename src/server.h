#ifndef __batyr_server_h__
#define __batyr_server_h__

#include <Poco/Util/ServerApplication.h>

#include <string>
#include <vector>

namespace Batyr {

    class Server : public Poco::Util::ServerApplication
    {
        protected:
            int main(const std::vector<std::string> &);

            /** initialize the logging system */
            void initLogging();
    };

};

#endif // __batyr_server_h__
