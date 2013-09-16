#ifndef __batyr_job_h__
#define __batyr_job_h__

#include <string>
#include <iostream>

namespace Batyr 
{

    class Job
    {
        private:
            std::string errorMessage;
            std::string id;
            
        public:
            Job();

            friend std::ostream& operator<< (std::ostream& , const Job&);

            void setErrorMessage(const std::string & em) 
            {
                errorMessage = em;
            }

            std::string toString() const;

    };

    std::ostream& operator<< (std::ostream& , const Job&);


};


#endif // __batyr_job_h__
