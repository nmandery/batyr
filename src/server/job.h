#ifndef __batyr_job_h__
#define __batyr_job_h__

#include "../lib/rapidjson/document.h"

#include <string>
#include <iostream>
#include <memory>
#include <chrono>


namespace Batyr 
{

    class Job
    {
        public:
            Job();

            friend std::ostream& operator<< (std::ostream& , const Job&);

            std::string getId()
            {
                return id;
            }

            std::chrono::system_clock::time_point getTimeAdded()
            {
                return timeAdded;
            }

            void setErrorMessage(const std::string & em) 
            {
                errorMessage = em;
                status = FAILED;
            }

            /** return the object as a json string */
            std::string toString() const;

            /** push the contents of the object into rapidjson document or value */
            void toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const;

            typedef std::shared_ptr<Job> Ptr;

            enum Status {
                QUEUED,
                IN_PROCESS,
                FINISHED,
                FAILED
            };

        private:
            std::string errorMessage;
            std::string id;
            Job::Status status;
            std::chrono::system_clock::time_point timeAdded;

 
    };

    std::ostream& operator<< (std::ostream& , const Job&);

};


#endif // __batyr_job_h__
