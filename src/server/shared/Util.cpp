#include "Util.h"

#include <boost/algorithm/string.hpp>

std::string Util::Date(const char* format, bool utc)
{
    boost::posix_time::ptime now;
    if (utc)
        now = boost::posix_time::second_clock::universal_time();
    else
        now = boost::posix_time::second_clock::local_time();

    std::stringstream ss;
    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet(format);
    ss.imbue(std::locale(std::cout.getloc(), facet));
    ss << now;

    return ss.str();
}

std::string Util::ToBase64(std::string input, bool linebreaks)
{
    uint32_t writePaddChars = (3 - input.length() % 3) % 3;
    
    if (linebreaks) {
        std::string base64(it_base64_lb_t(input.begin()), it_base64_lb_t(input.end()));
        base64.append(writePaddChars, '=');
        return base64;
    } else {
        std::string base64(it_base64_t(input.begin()), it_base64_t(input.end()));
        base64.append(writePaddChars, '=');
        return base64;
    }
}

std::string Util::FromBase64(std::string input)
{
    boost::algorithm::trim(input);
    uint32_t paddChars = count(input.begin(), input.end(), '=');
    std::replace(input.begin(), input.end(), '=', 'A'); // replace '=' by base64 encoding of '\0'
    std::string result(it_binary_t(input.begin()), it_binary_t(input.end())); // decode
    result.erase(result.end() - paddChars, result.end());  // erase padding '\0' characters
    return result;
}
