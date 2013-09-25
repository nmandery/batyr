#include "json.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <memory>


using namespace Batyr::Json;


std::string
Batyr::Json::stringify(rapidjson::Document & doc)
{
    rapidjson::StringBuffer buffer;

#ifdef _DEBUG
    // use pretty printed json when creating a debug build
    rapidjson::PrettyWriter< rapidjson::StringBuffer > writer(buffer);
#else
    rapidjson::Writer< rapidjson::StringBuffer > writer(buffer);
#endif

    doc.Accept(writer);

    return std::string(buffer.GetString());
}


std::string
Batyr::Json::stringify(std::chrono::system_clock::time_point tp)
{
    time_t tp_t = std::chrono::system_clock::to_time_t(tp);
    tm utc_tm = *std::gmtime(&tp_t);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%FT%TZ", &utc_tm);
    return std::string(buffer);
}


void
Batyr::Json::toValue(rapidjson::Value & _v, std::string _s,
            rapidjson::Document::AllocatorType & allocator)
{
    _v.SetString(_s.c_str(), _s.size(), allocator);
}

void
Batyr::Json::toValue(rapidjson::Value & _v, std::chrono::system_clock::time_point tp,
            rapidjson::Document::AllocatorType & allocator)
{
    toValue(_v, stringify(tp), allocator);
}
