#ifndef __batyr_jobstorage_h__
#define __batyr_jobstorage_h__

#include <Poco/Logger.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

#include "job.h"
#include "quitablequeue.h"


namespace Batyr
{

    typedef QuitableQueue<Job::Ptr> JobQueue;

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
            std::mutex modificationMutex;
            JobQueue queue;

        public:
            JobStorage();
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
