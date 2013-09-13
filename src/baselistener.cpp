#include "baselistener.h"

using namespace Geopoll;


BaseListener::BaseListener()
    : logger(Poco::Logger::get("BaseListener"))
{
    //poco_debug(logger, "Setting up listener");
}

BaseListener::~BaseListener()
{
    //poco_debug(logger, "Shutting down listener");
    //stop();
}
