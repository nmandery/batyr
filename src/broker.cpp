#include "broker.h"

using namespace GeoPoll;


Broker::Broker() 
    : logger(Poco::Logger::get("Broker"))
{
    poco_information(logger, "Setting up the broker");
    //logger = Poco::Logger::get("Broker");
}


Broker::~Broker()
{
    stop();
}


void
Broker::addListener(std::shared_ptr<Geopoll::BaseListener> listener_ptr)
{
    listeners.push_back(listener_ptr);
}


void
Broker::start()
{
    // start all listeners
    for(auto ilistener = listeners.begin(); ilistener != listeners.end() ; ++ilistener) {
        (*ilistener)->start();
    }
}


void
Broker::stop()
{
    // stopping all listeners
    /*
    for(auto ilistener = listeners.begin(); ilistener != listeners.end() ; ++ilistener) {
        (*ilistener)->stop();
    }
    */
}
