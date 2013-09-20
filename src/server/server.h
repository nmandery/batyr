#ifndef __batyr_server_h__
#define __batyr_server_h__

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/OptionSet.h>

#include <string>
#include <vector>

namespace Batyr {

    class Server : public Poco::Util::ServerApplication
    {
        protected:
            int main(const std::vector<std::string> &);

            /** initialize the logging system */
            void initLogging();

            /* Commandline argument parsing -----------*/

            bool _helpRequested = false;

            /** indicates that the configfile is found and free of errors */
            bool _configOk = true; 

            void defineOptions(Poco::Util::OptionSet & options);
            void handleHelp(const std::string& name, const std::string& value);
            void handleConfigfile(const std::string& name, const std::string& value);
            void displayHelp();

    };

};

#endif // __batyr_server_h__
