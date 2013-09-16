#include "joblist.h"

using namespace Batyr;

JobList::JobList()
    :   logger(Poco::Logger::get("Broker"))
{
    // TODO: start thread to cleanup finished jobs

}


void
JobList::addJob(Job::Ptr _job)
{
    // TODO
}


void
JobList::removeJob(std::string _id)
{
    // TODO
}


Job::Ptr 
JobList::getJob(std::string _id)
{
    // TODO
}


std::vector< Job::Ptr >
JobList::getOrderedJobs()
{
    // TODO
}
