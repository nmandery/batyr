#include "broker.h"

#include <thread>
#include <functional>
#include <stdexcept>


using namespace Batyr;


Broker::Broker() 
    :   logger(Poco::Logger::get("Broker")),
        jobs(std::make_shared<JobList>()),
        zmq_cx(0)
{
    poco_debug(logger, "Setting up the broker");
    
    zmq_cx = new zmq::context_t(1);
    if (zmq_cx == nullptr) {
        throw std::runtime_error("Could not intialize 0mq context");
    }
}


Broker::~Broker()
{
    stop();

    if (zmq_cx != nullptr) {
        delete zmq_cx;
    }
}


void
Broker::addListener(std::shared_ptr<Batyr::BaseListener> listener_ptr)
{
    listener_ptr->setJobs(jobs);
    listeners.push_back(listener_ptr);
}


void
Broker::run()
{
    // start all listeners
    for(auto ilistener : listeners) {
        // http://stackoverflow.com/questions/10673585/start-thread-with-member-function
        std::thread lThread( std::bind(&Batyr::BaseListener::run, ilistener) );
        lThread.detach();
    }
}


void
Broker::stop()
{
    // stopping all listeners
    for(auto ilistener : listeners) {
        ilistener->stop();
    }
}
