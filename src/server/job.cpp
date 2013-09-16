#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

#include "../lib/rapidjson/prettywriter.h"
#include "../lib/rapidjson/stringbuffer.h"

#include "job.h"

using namespace Batyr;


Job::Job()
    : status(FAILED)
{
    // generate an UUID as id for the job
    Poco::UUIDGenerator & uuidGen = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID uuid(uuidGen.createRandom());
    id = uuid.toString();

}


void
Job::toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const
{
    targetValue.SetObject();
    targetValue.AddMember("id", id.c_str(), allocator);

    std::string statusString;
    switch (status) {
        case QUEUED:
            statusString = "queued";
            break;
        case IN_PROCESS:
            statusString = "in_process";
            break;
        case FINISHED:
            statusString = "finished";
            break;
        case FAILED:
            statusString = "failed";
            break;
    }
    targetValue.AddMember("status", statusString.c_str(), allocator);


    if (!errorMessage.empty()) {
        targetValue.AddMember("errorMessage", errorMessage.c_str(), allocator);
    }
}


std::string
Job::toString() const
{
    // usage of rapidjson:
    // http://www.thomaswhitton.com/blog/2013/06/27/json-c-plus-plus-examples/

    rapidjson::Document data;

    toJsonValue(data, data.GetAllocator());

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
