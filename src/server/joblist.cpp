#include <stdexcept>
#include <algorithm>

#include "joblist.h"


using namespace Batyr;

JobList::JobList()
    :   logger(Poco::Logger::get("JobList"))
{
    // TODO: start thread to cleanup finished jobs

}


void
JobList::addJob(Job::Ptr _job)
{
    poco_debug(logger, "Locking joblist to add job");
    std::lock_guard<std::mutex> lock(modificationMutex);

    jobMap[_job->getId()] = _job;
}


void
JobList::removeJob(std::string _id)
{
    poco_debug(logger, "Locking joblist to remove job");
    std::lock_guard<std::mutex> lock(modificationMutex);

    jobMap.erase(_id);
}


Job::Ptr
JobList::getJob(std::string _id)
{
    poco_debug(logger, "Getting job from JobList");

    auto foundJob = jobMap.find(_id);
    if (foundJob == jobMap.end()) {
        poco_error(logger, "Attempt to fetch a job from joblist which is not part of the list");
        throw std::out_of_range("job is not contained in joblist");
    }
    return foundJob->second;
}


std::vector< Job::Ptr >
JobList::getOrderedJobs()
{
    poco_debug(logger, "Locking joblist to return a ordered vector of jobs");
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
