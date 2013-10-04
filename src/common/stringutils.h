#ifndef __common_stringutils_h__
#define __common_stringutils_h__

#include <string>

namespace StringUtils 
{
    /**
     * trim all of the given characters from both sides of the string
     */
    std::string trim(const std::string &s, const std::string & characters = " \t\r\n");

};

#endif /* __common_stringutils_h__ */
