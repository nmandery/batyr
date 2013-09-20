#include "server/worker.h"

using namespace Batyr;


Worker::Worker(std::shared_ptr<JobStorage> _jobs)
    :   logger(Poco::Logger::get("Worker")),
        jobs(_jobs)
{
    poco_debug(logger, "Creating Worker");
}


Worker::~Worker()
{
    poco_debug(logger, "Destroying Worker");
}

void
Worker::run()
{
    while (true) {
        Job::Ptr job;
        bool got_job = jobs->pop(job);
        if (!got_job) {
            break;
        }
        poco_debug(logger, "Got job from queue");
    }
    poco_debug(logger, "leaving run method");
}
