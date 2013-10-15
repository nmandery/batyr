#ifndef __batyr_jobstorage_h__
#define __batyr_jobstorage_h__

#include <Poco/Logger.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "server/job.h"
#include "server/quitablequeue.h"


namespace Batyr
{

    typedef QuitableQueue<Job::Ptr> JobQueue;


    struct JobStats
    {
        size_t numQueuedJobs;
        size_t numFailedJobs;
        size_t numInProcessJobs;
        size_t numFinishedJobs;


        JobStats()
                :   numQueuedJobs(0),
                    numFailedJobs(0),
                    numInProcessJobs(0),
                    numFinishedJobs(0)
        {
        }

        typedef std::unique_ptr<JobStats> Ptr;
    };

    /**
     * TODO:
     *  * the internal map is not restricted in size. so there might occur a
     *    growth problem.
     */
    class JobStorage
    {
        private:
            Poco::Logger & logger;
            std::unordered_map< std::string, Job::Ptr > jobMap;
            std::mutex mapModificationMutex;
            JobQueue queue;

            std::thread cleanupThread;

            /**
             * mutex to signal the cleanupthread when to exit.
             * This class releases its lock on the mutex to signal
             * the cleanup thread to exit
             */
            std::timed_mutex cleanupExitMutex;

            /**
             * the max age in seconds finished jobs are allowed to have
             * before they are removed
             */
            std::chrono::duration<int> maxAgeDoneJobs;

        public:
            JobStorage(std::chrono::duration<int> _maxAgeDoneJobs);
            ~JobStorage();

            /**
             * add a job
             */
            void addJob(Job::Ptr _job);

            /**
             * remove a job by its id
             */
            void removeJob(std::string _id);

            /**
             * get a job by its id
             */
            Job::Ptr getJob(std::string _id);

            /**
             */
            JobStats::Ptr getStats();

            /**
             * get a list of jobs ordered by their timestamp
             */
            std::vector< Job::Ptr > getOrderedJobs();

            /**
             * enqueue a job and add it
             */
            void push(Job::Ptr _job);

            /**
             * block and wait until a new job is available
             */
            bool pop(Job::Ptr & _job);

            /**
             * number of elements waiting in the queue
             */
            size_t queueSize()
            {
                return queue.size();
            }

            void quit()
            {
                queue.quit();
            }
    };

};


#endif // __batyr_jobstorage_h__
