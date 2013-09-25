#include <Poco/ConsoleChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Logger.h>
#include <Poco/AutoPtr.h>
#include <Poco/Message.h>
#include <Poco/Exception.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionCallback.h>
#include <Poco/Util/HelpFormatter.h>

#include <stdexcept>
#include <iostream>

#include "server/server.h"
#include "server/http/listener.h"
#include "common/macros.h"
#include "server/broker.h"


using namespace Batyr;


Server::Server()
    :   Poco::Util::ServerApplication(),
        _helpRequested(false),
        _configOk(false)
{
}


int
Server::main(const std::vector<std::string> & args)
{
    UNUSED(args)

    // exit early if the user just wanted to see 
    // the help text
    if (_helpRequested) {
        return Poco::Util::Application::EXIT_OK;
    }

    // check the config file
    if (!_configOk) {
        return Poco::Util::Application::EXIT_USAGE;
    }

    initLogging();
    Poco::Logger & logger = Poco::Logger::get("Server"); 

    try {
        Broker broker(configuration);

        auto httplistener_ptr = std::make_shared<Batyr::Http::Listener>(configuration);
        broker.addListener(httplistener_ptr);
        broker.run();

        waitForTerminationRequest();  // wait for CTRL-C or kill

        broker.stop();
    }
    catch (Poco::Exception& e) {
        poco_error(logger, e.displayText());
        std::cerr << e.displayText() << std::endl;

        return Poco::Util::Application::EXIT_SOFTWARE;
    }
    catch (std::exception& e) {
        poco_error(logger, e.what());
        std::cerr << e.what() << std::endl;

        return Poco::Util::Application::EXIT_SOFTWARE;
    }
    return Poco::Util::Application::EXIT_OK;
};


void
Server::initLogging()
{
    // initialize the logging system
    Poco::AutoPtr<Poco::ConsoleChannel> pCons(new Poco::ConsoleChannel);
    Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter);

    // formatting patterns 
    // see http://www.appinf.com/docs/poco/Poco.PatternFormatter.html
    pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S [%p] [%s] %t");
    Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, pCons));
    Poco::Logger::root().setChannel(pFC);

#ifdef _DEBUG
    // allow logging on max. level in debug builds
    Poco::Logger::root().setLevel(Poco::Message::PRIO_DEBUG);
#else
    Poco::Logger::root().setLevel(Poco::Message::PRIO_INFORMATION);
#endif

}


void
Server::defineOptions(Poco::Util::OptionSet & options)
{
    Poco::Util::ServerApplication::defineOptions(options);

    options.addOption( Poco::Util::Option("help", "h", "display help information")
            .required(false)
            .repeatable(false)
            .callback( Poco::Util::OptionCallback<Server>( this, &Server::handleHelp )));

    options.addOption( Poco::Util::Option("configfile", "c", "path to the config file")
            .required(true)
            .repeatable(false)
            .argument("file")
            .callback( Poco::Util::OptionCallback<Server>( this, &Server::handleConfigfile )));
}


void
Server::displayHelp()
{
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("-c CONFIGFILE [OPTIONS]");
    helpFormatter.setHeader("TODO ---- write some short description here");
    helpFormatter.format(std::cout);

}


void
Server::handleHelp(const std::string& name, const std::string& value)
{
    UNUSED(name)
    UNUSED(value)

    _helpRequested = true;
    displayHelp();
    stopOptionsProcessing();
}


void
Server::handleConfigfile(const std::string& name, const std::string& value)
{
    UNUSED(name)

    try {
        configuration = std::make_shared<Configuration>(value);
        _configOk = true;
    }
    catch (Batyr::ConfigurationError &e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        _configOk = false;
        stopOptionsProcessing();
    }
}
