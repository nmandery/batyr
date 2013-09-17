#ifndef __batyr_json_h__
#define __batyr_json_h__

#include "../lib/rapidjson/document.h"

#include <string>
#include <chrono>

namespace Batyr 
{
namespace Json
{

    /** helper function to convert rapidjson documents to strings */
    std::string stringify(rapidjson::Document & doc); 

    /** convert a timestamp to a JSON timestamp */
    std::string stringify(std::chrono::system_clock::time_point);

};
};

#endif // __batyr_json_h__
