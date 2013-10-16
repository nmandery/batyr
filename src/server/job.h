#ifndef __batyr_job_h__
#define __batyr_job_h__

#include "rapidjson/document.h"

#include <string>
#include <iostream>
#include <memory>
#include <chrono>


namespace Batyr
{

    class Job
    {
        public:
            /** type of job */
            enum Type {
                PULL,
                REMOVE_BY_ATTRIBUTES
            };

            enum Status {
                QUEUED,
                IN_PROCESS,
                FINISHED,
                FAILED
            };

            Job(Job::Type);

            typedef std::shared_ptr<Job> Ptr;

            void setStatus(Status _status)
            {
                status = _status;
                if (isDone()) {
                    timeFinished = std::chrono::system_clock::now();
                }
            }

            std::string getId()
            {
                return id;
            }

            std::chrono::system_clock::time_point getTimeAdded()
            {
                return timeAdded;
            }

            std::chrono::system_clock::time_point getTimeFinished()
            {
                return timeFinished;
            }

            void setMessage(const std::string & m)
            {
                message = m;
            }

            std::string getLayerName() const
            {
                return layerName;
            }

            std::string getFilter() const
            {
                return filter;
            }

            Job::Status getStatus() const
            {
                return status;
            }

            /**
             * true if the job is, successful or not, finished
             */
            bool isDone() const
            {
                return (status == FINISHED) || (status == FAILED);
            }

            /**
             * fill the objects members from a JSON string
             *
             * will throw a std::invalid_argument if things are not going
             * to well.
             */
            void fromString(std::string);

            /** push the contents of the object into rapidjson document or value */
            void toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const;


            void setStatistics(int _numPulled, int _numCreated, int _numUpdated, int _numDeleted)
            {
                numPulled = _numPulled;
                numCreated = _numCreated;
                numUpdated = _numUpdated;
                numDeleted = _numDeleted;
            }

            Job::Type getType() const
            {
                return type;
            }

        private:
            Job::Type type;
            std::string message;
            std::string layerName;
            std::string filter;
            std::string id;
            Job::Status status;
            std::chrono::system_clock::time_point timeAdded;
            std::chrono::system_clock::time_point timeFinished;

            // statistics how many rows have been modified
            int numCreated;
            int numUpdated;
            int numDeleted;
            int numPulled;


    };

    std::ostream& operator<< (std::ostream& , const Job&);

};


#endif // __batyr_job_h__
