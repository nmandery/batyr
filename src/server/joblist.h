#ifndef __batyr_joblist_h__
#define __batyr_joblist_h__

#include <Poco/Logger.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

#include "job.h"


namespace Batyr
{

    /**
     * TODO:
     *  * the internal map is not restricted in size. so there might occur a
     *    growth problem.
     */
    class JobList
    {
        private:
            Poco::Logger & logger;
            std::unordered_map< std::string, Job::Ptr > jobMap;
            std::mutex modificationMutex;

        public:
            JobList();

            /** add a job */
            void addJob(Job::Ptr _job);

            /** remove a job by its id */
            void removeJob(std::string _id);

            /** get a job by its id */
            Job::Ptr getJob(std::string _id);

            /** get a list of jobs ordered by their timestamp */
            std::vector< Job::Ptr > getOrderedJobs();
    };

};


#endif // __batyr_joblist_h__
