#include "server/error.h"
#include "server/json.h"


using namespace Batyr;


Error::Error(const std::string msg)
    :   message(msg)
{
}

void
Error::toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const
{
    targetValue.SetObject();

    rapidjson::Value vMessage;
    Batyr::Json::toValue(vMessage, message, allocator);
    targetValue.AddMember("message", vMessage, allocator);
}


std::ostream&
Batyr::operator<< (std::ostream& stream, const Error& r)
{
    stream << Batyr::Json::toJson(r);
    return stream;
}


