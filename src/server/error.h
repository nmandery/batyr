#ifndef __batyr_error_h__
#define __batyr_error_h__

#include "rapidjson/document.h"

#include <string>
#include <iostream>
#include <memory>

namespace Batyr
{
    class Error
    {
        public:
            Error(const std::string msg);

            typedef std::shared_ptr<Error> Ptr;

            void setMessage(const std::string & m)
            {
                message = m;
            }

            std::string getMessage() const
            {
                return message;
            }

            /** push the contents of the object into rapidjson document or value */
            void toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const;

        private:
            std::string message;
    };

    std::ostream& operator<< (std::ostream& , const Error&);
};

#endif // __batyr_error_h__
