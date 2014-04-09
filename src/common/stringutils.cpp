#include "stringutils.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <sstream>
#include <vector>

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

size_t 
StringUtils::levenshteinDistance(const std::string &s1, const std::string &s2)
{
    const size_t m(s1.size());
    const size_t n(s2.size());
 
    if( m==0 ) {
        return n;
    }
    if( n==0 ) {
        return m;
    }
 
    std::vector<size_t> costs;
    costs.reserve(n+1);
    for( size_t k=0; k<=n; k++ ) {
        costs[k] = k;
    }
 
    size_t i = 0;
    for ( std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i ) {
        costs[0] = i+1;
        size_t corner = i;
 
        size_t j = 0;
        for ( std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j ) {
            size_t upper = costs[j+1];
            if ( *it1 == *it2 ) {
                costs[j+1] = corner;
            }
            else {
                size_t t(upper<corner?upper:corner);
                costs[j+1] = (costs[j]<t?costs[j]:t)+1;
            }
            corner = upper;
        }
    }

    return costs[n];
}
