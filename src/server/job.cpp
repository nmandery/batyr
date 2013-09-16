#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

#include "job.h"
#include "../lib/rapidjson/document.h"
#include "../lib/rapidjson/prettywriter.h"
#include "../lib/rapidjson/stringbuffer.h"

using namespace Batyr;


Job::Job()
{
    // generate an UUID as id for the job
    Poco::UUIDGenerator uuidGen;
    Poco::UUID uuid(uuidGen.createRandom());
    id = uuid.toString();

}

std::string
Job::toString() const
{
    // usage of rapidjson:
    // http://www.thomaswhitton.com/blog/2013/06/27/json-c-plus-plus-examples/

    rapidjson::Document data;
    data.SetObject();

    data.AddMember("id", id.c_str(), data.GetAllocator());

    if (!errorMessage.empty()) {
        data.AddMember("errorMessage", errorMessage.c_str(), data.GetAllocator());
        data.AddMember("success", false, data.GetAllocator());
    }
    else {
        data.AddMember("success", true, data.GetAllocator());
    }

    // stringify
    rapidjson::GenericStringBuffer< rapidjson::UTF8<> > buffer;
    rapidjson::Writer< rapidjson::GenericStringBuffer< rapidjson::UTF8<> > > writer(buffer);
    data.Accept(writer);

    return std::string(buffer.GetString());
}


std::ostream&
Batyr::operator<< (std::ostream& stream, const Job& r)
{
    stream << r.toString();

    return stream;
}
