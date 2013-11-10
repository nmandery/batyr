#include "stringutils.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <sstream>

using namespace StringUtils;

/**
 * trim all of the given characters from both sides of the string
 */
std::string
StringUtils::trim(const std::string &s, const std::string & characters)
{
    size_t sec_start = s.find_first_not_of(characters);
    size_t sec_end = s.find_last_not_of(characters);

    if (sec_start == std::string::npos) {
        sec_start = 0;
    }
    if (sec_end == std::string::npos) {
        sec_end = s.length()-1;
  }
    return s.substr(sec_start, sec_end - sec_start + 1);
}


std::string
StringUtils::join( const std::vector<std::string>& elements, const char* const separator)
{
    switch (elements.size())
    {
        case 0:
            return "";
        case 1:
            return elements[0];
        default:
            std::ostringstream os;
            std::copy(elements.begin(), elements.end()-1, std::ostream_iterator<std::string>(os, separator));
            os << *elements.rbegin();
            return os.str();
    }
}


std::vector<std::string>
StringUtils::split(const std::string &joined, const char seperator)
{
    std::vector<std::string> elements;
    std::string::const_iterator cur = joined.begin();
    std::string::const_iterator beg = joined.begin();
    while ( cur < joined.end() ) {
        if ( *cur == seperator ) {
            elements.push_back(std::string( beg , cur));
            beg = ++cur;
        }
        else {
            cur++;
        }
    }
    elements.push_back(std::string( beg , cur));
    return elements;
}


std::string
StringUtils::tolower(const std::string &s)
{
    std::string result;
    std::transform(s.begin(), s.end(), std::back_inserter(result), ::tolower);
    return std::move(result);
}
