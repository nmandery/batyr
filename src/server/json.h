#ifndef __batyr_json_h__
#define __batyr_json_h__

#include "../lib/rapidjson/document.h"

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
};
};

#endif // __batyr_json_h__
