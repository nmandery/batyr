#ifndef __common_stringutils_h__
#define __common_stringutils_h__

#include <string>
#include <vector>

namespace StringUtils 
{
    /**
     * trim all of the given characters from both sides of the string
     */
    std::string trim(const std::string &s, const std::string & characters = " \t\r\n");

    std::string join( const std::vector<std::string>& elements, const char* const separator);
};

#endif /* __common_stringutils_h__ */
