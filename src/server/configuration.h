#ifndef __batyr_configuration_h__
#define __batyr_configuration_h__

#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace Batyr
{
    
    struct Layer
    {
        std::string name;
        std::string dataSource;
        std::string description;

        typedef std::shared_ptr<Layer> Ptr;
    };


    class ConfigurationError : public std::runtime_error 
    {
        public:
            ConfigurationError(const std::string & message) 
                    : std::runtime_error(message)
            {
            };
    };


    class Configuration
    {
        public:
            Configuration(const std::string & configFile);

            /**
             * get a layer by its name.
             * throws ConfigurationError if it does not exist
             */
            Layer::Ptr getLayer(const std::string &) const;

            unsigned int getHttpPort() const
            {
                return http_port;
            }

            unsigned int getNumWorkerThreads() const
            {
                return num_worker_threads;
            }

            unsigned int getLayerCount() const
            {
                return layers.size();
            }

            unsigned int getMaxAgeDoneJobs() const
            {
                return max_age_done_jobs;
            }

            typedef std::shared_ptr<Configuration> Ptr;

        private:
            std::unordered_map<std::string, Layer::Ptr> layers;

            /* settings */
            unsigned int http_port;
            unsigned int num_worker_threads;
            unsigned int max_age_done_jobs;

            void parse(const std::string & configFile);
    };

};

#endif // __batyr_configuration_h__
