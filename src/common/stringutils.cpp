#include "stringutils.h"

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


