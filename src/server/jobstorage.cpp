#include <stdexcept>
#include <algorithm>

#include "jobstorage.h"


using namespace Batyr;

JobStorage::JobStorage(std::chrono::duration<int> _maxAgeDoneJobs)
    :   logger(Poco::Logger::get("JobStorage")),
        maxAgeDoneJobs( _maxAgeDoneJobs )
{
    // start thread to cleanup finished jobs
    cleanupExitMutex.lock();
    cleanupThread = std::thread([&jobMap, &logger, &maxAgeDoneJobs, &cleanupExitMutex, &mapModificationMutex](){
        // run every 10 seconds
        while (!cleanupExitMutex.try_lock_for( std::chrono::duration<int>( 10 ) )) {
            // perform the cleaning
            std::lock_guard<std::mutex> lock(mapModificationMutex);

            // calculate the min finishTime a job must have to be spared
            // from the cleaning
            auto minTime = std::chrono::system_clock::now() - maxAgeDoneJobs;
                // TODO:
            poco_debug(logger, "Starting to remove deprecated jobs");

            size_t numRemovedJobs = 0;
            for (auto it = jobMap.begin(), ite = jobMap.end(); it != ite;) {
                if (it->second->isDone()) {
                    if (it->second->getTimeFinished() < minTime) {
                        it = jobMap.erase(it);
                        numRemovedJobs++;
                    }
                    else {
                        ++it;
                    }
                }
                else {
                    ++it;
                }
            }
            poco_debug(logger, "Removed " + std::to_string(numRemovedJobs) + " deprecated jobs");
        }
        // locking succeded. time to exit
        cleanupExitMutex.unlock();
        poco_debug(logger, "Exiting cleanup thread");
    });

}

JobStorage::~JobStorage()
{
    // signal all waiting consumers we are done here
    queue.quit();

    // stop the cleanupthread
    cleanupExitMutex.unlock();
    cleanupThread.join();
}

void
JobStorage::addJob(Job::Ptr _job)
{
    poco_debug(logger, "Locking jobstorage to add job");
    std::lock_guard<std::mutex> lock(mapModificationMutex);

    jobMap[_job->getId()] = _job;
}


void
JobStorage::removeJob(std::string _id)
{
    poco_debug(logger, "Locking jobstorage to remove job");
    std::lock_guard<std::mutex> lock(mapModificationMutex);

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
    std::lock_guard<std::mutex> lock(mapModificationMutex);

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
