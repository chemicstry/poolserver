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

uint32 Util::Date()
{
    boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1)); 
    boost::posix_time::time_duration diff = now - epoch;
    return diff.total_seconds();
}

std::string Util::FS(const char *str, ...)
{
    va_list ap;
    va_start(ap, str);

    char text[MAX_FORMAT_LEN];
    vsnprintf(text, MAX_FORMAT_LEN, str, ap);

    va_end(ap);
    
    return std::string(text);
}

std::vector<std::string> Util::Explode(std::string input, std::string delim)
{
    std::vector<std::string> strs;
    boost::split(strs, input, boost::is_any_of(delim));
    return strs;
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

uint8 Util::ASCIIToHex(char ch)
{
    if (ch > 47 && ch < 58) // Number
        return ch - 48;
    else if (ch > 64 && ch < 71) // Uppercase
        return ch - 55;
    else if (ch > 96 && ch < 103) // Lowercase
        return ch - 87;
    else 
        return 0; // Invalid
}

BinaryData Util::ASCIIToBin(std::string str)
{
    BinaryData data;
    data.resize((str.size()+1)/2, 0);
    for (uint64 i = 0; i < str.size(); ++i) {
        if (i%2)
            data[i/2] += ASCIIToHex(str[i]);
        else
            data[i/2] += ASCIIToHex(str[i])*16;
    }
    return data;
}

std::string Util::BinToASCII(BinaryData data)
{
    std::string str;
    for (uint64 i = 0; i < data.size(); ++i)
    {
        str += "0123456789abcdef"[data[i]/16];
        str += "0123456789abcdef"[data[i]%16];
    }
    return str;
}

BinaryData Util::Reverse(BinaryData data)
{
    BinaryData out = data;
    std::reverse(out.begin(), out.end());
    return out;
}

BinaryData Util::Join(BinaryData v1, BinaryData v2)
{
    BinaryData v3 = v1;
    v3.insert(v3.end(), v2.begin(), v2.end());
    return v3;
}
