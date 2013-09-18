#include <stdexcept>
#include <algorithm>

#include "jobstorage.h"


using namespace Batyr;

JobStorage::JobStorage()
    :   logger(Poco::Logger::get("JobStorage"))
{
    // TODO: start thread to cleanup finished jobs

}

JobStorage::~JobStorage()
{
    // signal all waiting consumers we are done here
    queue.quit();
}

void
JobStorage::addJob(Job::Ptr _job)
{
    poco_debug(logger, "Locking jobstorage to add job");
    std::lock_guard<std::mutex> lock(modificationMutex);

    jobMap[_job->getId()] = _job;
}


void
JobStorage::removeJob(std::string _id)
{
    poco_debug(logger, "Locking jobstorage to remove job");
    std::lock_guard<std::mutex> lock(modificationMutex);

    jobMap.erase(_id);
}


Job::Ptr
JobStorage::getJob(std::string _id)
{
    poco_debug(logger, "Getting job from JobStorage");

    auto foundJob = jobMap.find(_id);
    if (foundJob == jobMap.end()) {
        poco_error(logger, "Attempt to fetch a job from jobstorage which is not part of the list");
        throw std::out_of_range("job is not contained in jobstorage");
    }
    return foundJob->second;
}


std::vector< Job::Ptr >
JobStorage::getOrderedJobs()
{
    poco_debug(logger, "Locking jobstorage to return a ordered vector of jobs");
    std::lock_guard<std::mutex> lock(modificationMutex);

    std::vector< Job::Ptr > orderedJobs;
    orderedJobs.reserve(jobMap.size());

    for (auto kv : jobMap) {
        orderedJobs.push_back(kv.second);
    }

    // sort the list of jobs
    std::sort(orderedJobs.begin(), orderedJobs.end(), [](Job::Ptr j1, Job::Ptr j2) {
        return j2->getTimeAdded() < j1->getTimeAdded();
    });

    return orderedJobs;
}


void
JobStorage::push(Job::Ptr _job)
{
    addJob(_job);
    queue.push(_job);
}


bool
JobStorage::pop(Job::Ptr & _job) {
    return queue.pop(_job);
}
