#ifndef __ini_iniparser_h__
#define __ini_iniparser_h__

#include <map>
#include <list>
#include <string>
#include <iostream>



namespace Ini
{

    struct Level
    {
        Level() : parent(NULL), depth(0) {}
        Level(Level* p) : parent(p), depth(0) {}

        typedef std::map<std::string, std::string> value_map_t;
        typedef std::map<std::string, Level> section_map_t;
        typedef std::list<value_map_t::const_iterator> values_t;
        typedef std::list<section_map_t::const_iterator> sections_t;
        value_map_t values;
        section_map_t sections;

        /**
         * original order in the ini file
         */
        values_t ordered_values;
        sections_t ordered_sections;

        Level* parent;
        size_t depth;

        const std::string& operator[](const std::string& name) { return values[name]; }
        Level& operator()(const std::string& name) { return sections[name]; }
    };

    class Parser
    {
        public:
            Parser(std::istream& f);
            Level& top() { return top_; }

        private:
            void parse(Level& l);
            void err(const char* s);

        private:
            Level top_;
            std::istream * stream_;
            std::string line_;
            size_t ln_;
    };


};

#endif // __ini_iniparser_h__
