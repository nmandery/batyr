#ifndef __batyr_json_h__
#define __batyr_json_h__

#include "rapidjson/document.h"

#include <string>
#include <chrono>

namespace Batyr 
{
namespace Json
{


    /**
     * helper function to convert rapidjson documents to strings
     */
    std::string stringify(rapidjson::Document & doc); 

    /**
     * convert a timestamp to a string containing a JSON timestamp
     */
    std::string stringify(std::chrono::system_clock::time_point);

    /**
     * copy the string's contents to rapidjson so the string can be
     * deallocated without rapidjsons data getting corrupted
     */
    void toValue(rapidjson::Value & _v, std::string _s, rapidjson::Document::AllocatorType & allocator); 

    void toValue(rapidjson::Value & _v, std::chrono::system_clock::time_point tp, rapidjson::Document::AllocatorType & allocator);

    /**
     * convert any class supporting the toJsonValue method
     * to a serialized JSON document
     **/
    template <class T>
    std::string toJson(T c)
    {
        rapidjson::Document data;
        c.toJsonValue(data, data.GetAllocator());
        return stringify(data);
    };

};
};

#endif // __batyr_json_h__
