#include "json.h"

#include "../lib/rapidjson/prettywriter.h"
#include "../lib/rapidjson/writer.h"
#include "../lib/rapidjson/stringbuffer.h"


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


