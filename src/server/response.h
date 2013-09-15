#ifndef __batyr_response_h__
#define __batyr_response_h__

#include <string>
#include <iostream>

namespace Batyr 
{

    class Response
    {
        protected:
            std::string errorMessage;
            
        public:
            friend std::ostream& operator<< (std::ostream& , const Response&);

            void setErrorMessage(const std::string & em) 
            {
                errorMessage = em;
            }

            std::string toString() const;

    };

    std::ostream& operator<< (std::ostream& , const Response&);


};


#endif // __batyr_response_h__
