#include "server/configuration.h"
#include "common/iniparser.h"
#include "common/macros.h"
#include "common/stringutils.h"

#include <iostream>
#include <fstream>
#include <algorithm>


using namespace Batyr;


/**
 * characters to trim from config values
 */
static const char trimChars[] = "'\"\r\n\t ";

Layer::Layer()
    :   allow_feature_deletion(false),
        ignore_failures(false)
{
}


Configuration::Configuration(const std::string & configFile)
    :   http_port(9090),        // default value
        num_worker_threads(2),  // default value
        max_age_done_jobs(600),  // default value
        loglevel(Poco::Message::PRIO_INFORMATION),  // default value
        logfile(""),
        use_persistent_connections(true)
{
    parse(configFile);
}

Layer::Ptr
Configuration::getLayer(const std::string & _layer) const
{
    Layer::Ptr layer;
    try {
        layer = layers.at(_layer);
    }
    catch (std::out_of_range) {
        throw ConfigurationError("Layer " + _layer + " not found");
    }
    return layer;
}


std::vector<Layer::Ptr>
Configuration::getOrderedLayers() const
{
    std::vector< Layer::Ptr > orderedLayers;
    orderedLayers.reserve(layers.size());

    for (auto const kv : layers) {
        orderedLayers.push_back(kv.second);
    }

    std::sort(orderedLayers.begin(), orderedLayers.end(), [](Layer::Ptr l1, Layer::Ptr l2) {
        return l2->name > l1->name;
    });

    return orderedLayers;
}


void
static throwUnknownSetting(const std::string & sectionName, const std::string & settingName)
{
    throw ConfigurationError("Unknown option \""+settingName+
                    "\" in section \""+sectionName+"\"");
}

static void
throwInvalidValue(const std::string & sectionName, const std::string & settingName,
        const std::string & value)
{
    throw ConfigurationError("Invalid value for \""+settingName+
                    "\" in section \""+sectionName+"\": \""+value+"\"");
}


static int
valueToInt(const std::string & s, bool & ok)
{
    int i = 0;
    try {
        i = std::stoi(StringUtils::trim(s, trimChars));
        ok = true;
    }
    catch (std::exception) {
        ok = false;
    }
    return i;
}

static bool
valueToBool(const std::string & s, bool & ok)
{
    std::string sTrimmed = StringUtils::tolower(StringUtils::trim(s, trimChars));

    bool result = false;
    if ((sTrimmed == "true") || (sTrimmed == "yes") || (sTrimmed == "1") || (sTrimmed == "y") || (sTrimmed == "t")) {
        ok = true;
        result = true;
    }
    else if ((sTrimmed == "false") || (sTrimmed == "no") || (sTrimmed == "0") || (sTrimmed == "n") || (sTrimmed == "f")) {
        ok = true;
        result = false;
    }
    else {
        ok = false;
    }
    return result;
}

void
Configuration::parse(const std::string & configFile)
{
    std::ifstream ifs;
    ifs.open(configFile);
    if (!ifs.is_open()) {
        throw ConfigurationError("Could not open configfile at "+configFile);
    }

    try {

        Ini::Parser parser(ifs);
        Ini::Level toplevel = parser.top();

        bool ok = true;
        // parse all section seperate
        for(auto const sectionPair : toplevel.sections) {
            if (sectionPair.first == "HTTP") {
                for(auto const valuePair : sectionPair.second.values) {
                    if (valuePair.first == "port") {
                        http_port = valueToInt(valuePair.second, ok);
                        if (!ok) {
                            throwInvalidValue(sectionPair.first,
                                        valuePair.first,
                                        valuePair.second);
                        }
                    }
                    else {
                        throwUnknownSetting(sectionPair.first, valuePair.first);
                    }
                }
            }
            else if (sectionPair.first == "MAIN") {
                for(auto const valuePair : sectionPair.second.values) {
                    if (valuePair.first == "num_worker_threads") {
                        int _num_worker_threads = valueToInt(valuePair.second, ok);
                        if (!ok) {
                            throwInvalidValue(sectionPair.first,
                                        valuePair.first,
                                        valuePair.second);
                        }
                        if (_num_worker_threads < 1) {
                            throw ConfigurationError("At least one worker thread is required.");
                        }
                        num_worker_threads = _num_worker_threads;
                    }
                    else if (valuePair.first == "max_age_done_jobs") {
                        int _max_age_done_jobs = valueToInt(valuePair.second, ok);
                        if (!ok) {
                            throwInvalidValue(sectionPair.first,
                                        valuePair.first,
                                        valuePair.second);
                        }
                        if (_max_age_done_jobs < 1) {
                            throw ConfigurationError("max_age_done_jobs must be a positive value.");
                        }
                        max_age_done_jobs = _max_age_done_jobs;
                    }
                    else if (valuePair.first == "dsn") {
                        db_connection_string = StringUtils::trim(valuePair.second, trimChars);
                    }
                    else if (valuePair.first == "use_persistent_connections") {
                        bool ok = false;
                        bool _use_persistent_connections = valueToBool(valuePair.second, ok);
                        if (!ok) {
                            throw ConfigurationError("use_persistent_connections must be a boolean value (true,false,yes,no,1,0).");
                        }
                        use_persistent_connections = _use_persistent_connections;
                    }
                    else {
                        throwUnknownSetting(sectionPair.first, valuePair.first);
                    }
                }
            }
            else if (sectionPair.first == "LAYERS") {
                for(auto const layerSectionPair : sectionPair.second.sections) {
                    auto layer = std::make_shared<Layer>();
                    layer->name = layerSectionPair.first;

                    // collect layer infos
                    for(auto const layerValuePair: layerSectionPair.second.values) {
                        if (layerValuePair.first == "description") {
                            layer->description = layerValuePair.second;
                        }
                        else if (layerValuePair.first == "source") {
                            layer->source = layerValuePair.second;
                        }
                        else if (layerValuePair.first == "source_layer") {
                            layer->source_layer = layerValuePair.second;
                        }
                        else if (layerValuePair.first == "target_table_schema") {
                            layer->target_table_schema = layerValuePair.second;
                        }
                        else if (layerValuePair.first == "target_table_name") {
                            layer->target_table_name = layerValuePair.second;
                        }
                        else if (layerValuePair.first == "filter") {
                            layer->filter = layerValuePair.second;
                        }
                        else if (layerValuePair.first == "allow_feature_deletion") {
                            bool ok = false;
                            bool _allow_feature_deletion = valueToBool(layerValuePair.second, ok);
                            if (!ok) {
                                throw ConfigurationError("allow_feature_deletion must be a boolean value (true,false,yes,no,1,0).");
                            }
                            layer->allow_feature_deletion = _allow_feature_deletion;
                        }
                        else if (layerValuePair.first == "ignore_failures") {
                            bool ok = false;
                            bool _ignore_failures = valueToBool(layerValuePair.second, ok);
                            if (!ok) {
                                throw ConfigurationError("ignore_failures must be a boolean value (true,false,yes,no,1,0).");
                            }
                            layer->ignore_failures = _ignore_failures;
                        }
                        else if (layerValuePair.first == "primary_key_columns") {
                            auto pk_columns_untrimmed = StringUtils::split(layerValuePair.second, ',');
                            for(auto const pk_column_untrimmed : pk_columns_untrimmed) {
                                auto trimmed =  StringUtils::trim(pk_column_untrimmed, trimChars);
                                if (!trimmed.empty()) {
                                    std::cout << trimmed << std::endl;
                                    layer->primary_key_columns.push_back(trimmed);
                                }
                            }
                        }
                        else {
                            throwUnknownSetting(layerSectionPair.first, layerValuePair.first);
                        }
                    }

                    // check for missing mantatory settings
#define CHECK_STR_SETTING(LAYER, SETTING) if (LAYER->SETTING.empty()) { \
                        throw ConfigurationError("Layer \"" + LAYER->name + "\" is missing the \"" STRINGIFY(SETTING) "\" setting"); \
                    }

                    CHECK_STR_SETTING(layer, source);
                    CHECK_STR_SETTING(layer, source_layer);
                    CHECK_STR_SETTING(layer, target_table_name);
                    CHECK_STR_SETTING(layer, target_table_schema);

#undef CHECK_STR_SETTING

                    layers[layer->name] = layer;
                }
            }
            else if (sectionPair.first == "LOGGING") {
                for(auto const valuePair : sectionPair.second.values) {
                    if (valuePair.first == "loglevel") {
                        std::string loglevelStr = StringUtils::tolower(StringUtils::trim(valuePair.second, trimChars));
                        if (loglevelStr == "error") {
                            loglevel = Poco::Message::PRIO_ERROR;
                        }
                        else if (loglevelStr == "warning") {
                            loglevel = Poco::Message::PRIO_WARNING;
                        }
                        else if (loglevelStr == "information") {
                            loglevel = Poco::Message::PRIO_INFORMATION;
                        }
                        // poco only enables this loglevel in debug builds
                        else if (loglevelStr == "debug") {
#ifdef _DEBUG
                            loglevel = Poco::Message::PRIO_DEBUG;
#else
                            std::cerr << "The loglevel \"debug\" is only available in debug build. Using level \"information\" instead." << std::endl;
                            loglevel = Poco::Message::PRIO_INFORMATION;
#endif
                        }
                        else {
                            throw ConfigurationError("Unknown loglevel: \"" + valuePair.second + "\"");
                        }
                    }
                    else if (valuePair.first == "logfile") {
                        logfile = StringUtils::trim(valuePair.second, trimChars);
                    }
                    else {
                        throwUnknownSetting(sectionPair.first, valuePair.first);
                    }
                }
            }
            else {
                throw ConfigurationError("Unknown section: \""+sectionPair.first+"\"");
            }
        }

        // check for missing mantatory settings
        if (db_connection_string.empty()) {
            throw ConfigurationError("Missing dsn configuration to connect to postgresql");
        }

    }
    catch( std::runtime_error &e) {
        throw ConfigurationError(e.what());
    }
    catch( std::exception ) {
        throw ConfigurationError("Could not parse configuraton file");
    }
}

