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

    std::vector<std::string> split(const std::string &joined, const char seperator);

    /**
     * convert a string to lowercase
     */
    std::string tolower(const std::string &s);

    size_t levenshteinDistance(const std::string &s1, const std::string &s2);

    void replaceAll(std::string &subject, const std::string &from, const std::string &to);

};

#endif /* __common_stringutils_h__ */
