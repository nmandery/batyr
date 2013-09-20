#include "server/baselistener.h"

using namespace Batyr;


BaseListener::BaseListener(Configuration::Ptr _configuration)
    :   logger(Poco::Logger::get("BaseListener")),
        configuration(_configuration)
        
{
    //poco_debug(logger, "Setting up listener");
}

BaseListener::~BaseListener()
{
    //poco_debug(logger, "Shutting down listener");
    //stop();
}
