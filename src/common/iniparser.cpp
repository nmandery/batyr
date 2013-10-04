#include "iniparser.h"
#include "stringutils.h"

#include <stdexcept>
#include <cstring>
#include <sstream>


using namespace Ini;


Parser::Parser(std::istream& f) 
    :   stream_(&f), ln_(0)
{
    parse(top_); 
}


inline void
Parser::err(const char* s)
{
    std::stringstream msgstream;
    msgstream << s << " on line " << ln_;
    throw std::runtime_error(msgstream.str());
}


void
Parser::parse(Level& l)
{
    while (std::getline(*stream_, line_)) {
        ++ln_;
        line_ = StringUtils::trim(line_);

        // skip lines without content
        if (line_.empty()) {
            continue; 
        }

        // skip comments
        if (line_[0] == '#' || line_[0] == ';') {
            continue;
        }

        // line containing a section marker
        if (line_[0] == '[') {
            size_t depth = 0;
            std::string sectionName;

            // find the name of the section 
            for (; depth < line_.length(); ++depth) {
                if (line_[depth] != '[') {
                    break;
                }
            }
            sectionName = StringUtils::trim(line_, "\n\t []\r");


            Level* lp = NULL;
            Level* parent = &l;

            if (depth > l.depth + 1) {
                err("section with wrong depth");
            }

            if (l.depth == depth-1) {
                lp = &l.sections[sectionName];
            }
            else {
                lp = l.parent;
                size_t n = l.depth - depth;
                for (size_t i = 0; i < n; ++i) lp = lp->parent;
                parent = lp;
                lp = &lp->sections[sectionName];
            }
            if (lp->depth != 0) {
                err("duplicate section name on the same level");
            }
            if (!lp->parent) {
                lp->depth = depth;
                lp->parent = parent;
            }
            parent->ordered_sections.push_back(parent->sections.find(sectionName));
            parse(*lp);
        }
        // key-value pair
        else {
            size_t n = line_.find('=');
            if (n == std::string::npos) {
                err("no '=' found");
            }
            std::pair<Level::value_map_t::const_iterator, bool> res =
                l.values.insert(std::make_pair(
                            StringUtils::trim(line_.substr(0, n)),
                            StringUtils::trim(line_.substr(n+1, line_.length()-n-1))
                            ));
            if (!res.second) {
                err("duplicated key found");
            }
            l.ordered_values.push_back(res.first);
        }
    }
}

