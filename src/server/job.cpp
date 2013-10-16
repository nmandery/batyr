#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

#include <ctime>
#include <stdexcept>
#include <algorithm>

#include "server/job.h"
#include "server/json.h"
#include "common/stringutils.h"

#include "rapidjson/document.h"

using namespace Batyr;


Job::Job(Job::Type _type)
    :   type(_type),
        status(QUEUED),
        timeAdded(std::chrono::system_clock::now()),
        numCreated(0),
        numUpdated(0),
        numDeleted(0),
        numPulled(0)
{
    // generate an UUID as id for the job
    Poco::UUIDGenerator & uuidGen = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID uuid(uuidGen.createRandom());
    id = uuid.toString();

    // remove all dashes from the string for nicer looking URLs ;)
    // if the syntax of the id is changed, some ot the url parsing in 
    // the http/httprequesthandlerfactory.cpp needs to be changed as well
    std::remove(id.begin(), id.end(), '-');
}


void
Job::toJsonValue(rapidjson::Value & targetValue, rapidjson::Document::AllocatorType & allocator) const
{
    targetValue.SetObject();
    targetValue.AddMember("id", id.c_str(), allocator);

    rapidjson::Value vTimeAdded;
    Batyr::Json::toValue(vTimeAdded, timeAdded, allocator);
    targetValue.AddMember("timeAdded", vTimeAdded, allocator);

    if (isDone()) {
        rapidjson::Value vTimeFinished;
        Batyr::Json::toValue(vTimeFinished, timeFinished, allocator);
        targetValue.AddMember("timeFinished", vTimeFinished, allocator);
    }

    const char * typeString;
    switch(type) {
        case PULL:
            typeString = "pull";
            break;
        case REMOVE_BY_ATTRIBUTES:
            typeString = "remove-by-attributes";
            break;
    }
    rapidjson::Value vTypeString;
    Batyr::Json::toValue(vTypeString, typeString, allocator);
    targetValue.AddMember("type", vTypeString, allocator);

    const char * statusString;
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
    rapidjson::Value vStatusString;
    Batyr::Json::toValue(vStatusString, statusString, allocator);
    targetValue.AddMember("status", vStatusString, allocator);

    rapidjson::Value vLayerName;
    Batyr::Json::toValue(vLayerName, layerName, allocator);
    targetValue.AddMember("layerName", vLayerName, allocator);

    if (type == PULL) {
        rapidjson::Value vFilter;
        Batyr::Json::toValue(vFilter, filter, allocator);
        targetValue.AddMember("filter", vFilter, allocator);
    }
    else if (type == REMOVE_BY_ATTRIBUTES) {
        rapidjson::Value vAttributeSets;
        vAttributeSets.SetArray();
        vAttributeSets.Reserve(attributeSets.size(), allocator);

        for(const auto & attibuteSet : attributeSets) {
            rapidjson::Value vAttributeSet;
            vAttributeSet.SetObject();

            for(const auto & attributePair : attibuteSet) {
                rapidjson::Value vAttributeValue;
                if (attributePair.second.isNull()) {
                    vAttributeValue.SetNull();
                }
                else {
                    Batyr::Json::toValue(vAttributeValue, attributePair.second.get(), allocator);
                }
                vAttributeSet.AddMember(attributePair.first.c_str(), vAttributeValue, allocator);
            }
            vAttributeSets.PushBack(vAttributeSet, allocator);
        }

        targetValue.AddMember("attributeSets", vAttributeSets, allocator);
    }

    rapidjson::Value vMessage;
    Batyr::Json::toValue(vMessage, message, allocator);
    targetValue.AddMember("message", vMessage, allocator);


    targetValue.AddMember("numCreated", numCreated, allocator);
    targetValue.AddMember("numUpdated", numUpdated, allocator);
    targetValue.AddMember("numDeleted", numDeleted, allocator);
    targetValue.AddMember("numPulled", numPulled, allocator);
}


void
Job::fromString(std::string data)
{
    rapidjson::Document doc;
    doc.Parse<0>( data.c_str() );

    if (doc.HasParseError()) {
        throw std::invalid_argument("Invalid JSON data");
    }
    if (!doc.IsObject()) {
        throw std::invalid_argument("JSON data should be an object");
    }
    if (!doc.HasMember("layerName")) {
        throw std::invalid_argument("Missing key layerName in JSON object");
    }
    if (!doc["layerName"].IsString()) {
        throw std::invalid_argument("Key layerName should be a string");
    }
    layerName = doc["layerName"].GetString();

    if (doc.HasMember("filter")) {
        if (!doc["filter"].IsString()) {
            throw std::invalid_argument("Key filter should be a string");
        }
        filter = StringUtils::trim(doc["filter"].GetString());
    }

    if (doc.HasMember("attributeSets")) {
        auto & vAttributeSets = doc["attributeSets"];
        if (!vAttributeSets.IsArray()) {
            throw std::invalid_argument("Key attributeSets should be an array");
        }
        for (rapidjson::SizeType i = 0; i < vAttributeSets.Size(); i++) {
            auto & vAttributeSet = vAttributeSets[i];
            if (!vAttributeSet.IsObject()) {
                throw std::invalid_argument("Element number " + std::to_string(i)
                                + " in key attributeSets must be an object");
            }

            Job::AttributeSet attributeSet;
            for (rapidjson::Value::ConstMemberIterator it = vAttributeSet.MemberBegin(); it != vAttributeSet.MemberEnd(); ++it) {
                if (!it->name.IsString()) {
                    // should not happen, but you never know
                    throw std::invalid_argument("Object keys must be strings");
                }
                auto & attributeValue = attributeSet[it->name.GetString()];

                if (it->value.IsNull()) {
                    attributeValue.setIsNull(true);
                }
                else if (it->value.IsString()) {
                    attributeValue.setIsNull(false);
                    attributeValue.set(it->value.GetString());
                }
                else {
                    throw std::invalid_argument("Attibute values must be strings or null");
                }
            }

            if (attributeSet.size() > 0) {
                attributeSets.push_back(std::move(attributeSet));
            }
        }
    }
}



std::ostream&
Batyr::operator<< (std::ostream& stream, const Job& r)
{
    stream << Batyr::Json::toJson(r);

    return stream;
}


