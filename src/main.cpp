#include "Poco/ConsoleChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "Poco/Message.h"

#include "broker.h"
#include "config.h"
#include "httplistener.h"


using namespace GeoPoll;


// PROTOTYPES
void init_logging();


/** initialize the logging system */
void
init_logging()
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

int
main(int argc, char** argv)
{
    init_logging();

    GeoPoll::Broker broker;

    auto httplistener_ptr = std::make_shared<Geopoll::HttpListener>();
    broker.addListener(httplistener_ptr);
    broker.start();

    return 0;
}
