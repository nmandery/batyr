#include "broker.h"
#include "worker.h"

#include <thread>
#include <functional>
#include <stdexcept>


using namespace Batyr;


Broker::Broker() 
    :   logger(Poco::Logger::get("Broker")),
        jobs(std::make_shared<JobStorage>())
{
    poco_debug(logger, "Setting up the broker");
}


Broker::~Broker()
{
    stop();
}


void
Broker::addListener(std::shared_ptr<Batyr::BaseListener> listener_ptr)
{
    listener_ptr->setJobs(jobs);
    listeners.push_back(listener_ptr);
}


void
Broker::run(size_t _numWorkers)
{
    // start all workers
    poco_debug(logger, "Starting " + std::to_string(_numWorkers) + " worker threads");
    for(size_t nW = 0; nW < _numWorkers; nW++) {
        auto worker = std::unique_ptr<Worker>(new Worker(jobs));
        auto workerThread = std::make_shared<std::thread>( 
                std::bind(&Worker::run, std::move(worker)) 
        );
        workerThreads.push_back( workerThread );
    }

    // start all listener threads
    poco_debug(logger, "Starting " + std::to_string(listeners.size()) + " listener threads");
    for(auto ilistener : listeners) {
        // http://stackoverflow.com/questions/10673585/start-thread-with-member-function
        auto listenerThread = std::make_shared<std::thread>( 
                std::bind(&Batyr::BaseListener::run, ilistener)
        );
        listenerThreads.push_back( listenerThread );
    }
}


void
Broker::stop()
{
    // stopping all listeners
    if (!listeners.empty()) {
        for(auto ilistener : listeners) {
            ilistener->stop();
        }

        // wait until the listeners have shut down
        poco_information(logger, "Waiting for listeners to shut down");
        for(auto listenerThread: listenerThreads) {
            listenerThread->join();
        }
        listenerThreads.clear();
    }

    // quit the jobs. this will ttigger all attached workers to
    // exit
    jobs->quit();

    // wait until all workers are finished
    if (!workerThreads.empty()) {
        poco_information(logger, "Waiting for worker threads to finish");
        for(auto workerThread: workerThreads) {
            workerThread->join();
        }
        workerThreads.clear();
    }
}
