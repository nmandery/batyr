#include "server/error.h"
#include "server/json.h"


using namespace Batyr;


Error::Error(const std::string msg)
    :   errorMessage(msg)
{
}

void
Error::toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const
{
    targetValue.SetObject();

    rapidjson::Value vErrorMessage;
    Batyr::Json::toValue(vErrorMessage, errorMessage, allocator);
    targetValue.AddMember("errorMessage", vErrorMessage, allocator);
}


std::string
Error::toString() const
{
    rapidjson::Document data;
    toJsonValue(data, data.GetAllocator());
    return Batyr::Json::stringify(data);
}



std::ostream&
Batyr::operator<< (std::ostream& stream, const Error& r)
{
    stream << r.toString();
    return stream;
}


