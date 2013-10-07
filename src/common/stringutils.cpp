#include "stringutils.h"

#include <algorithm>
#include <sstream>
#include <iterator>

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

/*
void TextUtils::Split( const char pChr , std::vector<std::string> &pRet , const std::string &pPath )
{
    std::string::const_iterator cur = pPath.begin();
    std::string::const_iterator beg = pPath.begin();
    bool added=false;
    while ( cur < pPath.end() )
    {
        if ( *cur == pChr )
        {
            pRet.insert( pRet.end() , std::string( beg , cur) );
            beg = ++cur;
            added=true;
        }
        else
        {
            cur++;
        }
    }

    pRet.insert( pRet.end() , std::string( beg , cur) );
}
*/
