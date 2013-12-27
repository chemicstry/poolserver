#include "JSON.h"
#include "JSONReader.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "Log.h"

JSON JSON::FromString(std::string jsonstring)
{
    JSON node;
    JSONReader::Parse(jsonstring, node);
    return node;
}

std::string JSON::ToString()
{
    std::stringstream ss;
    Write(ss);
    std::string json = ss.str();
    
    // If it's array we have to wrap json into {}
    if (json[0] == '[')
        return "{" + json + "}";
    else
        return json;
}

void JSON::Write(std::stringstream& ss)
{
    switch (_type)
    {
        case JSON_NULL:
            ss << "null";
            break;
        case JSON_BOOL:
            if (GetBool())
                ss << "true";
            else
                ss << "false";
            break;
        case JSON_INTEGER:
            ss << GetInt();
            break;
        case JSON_DOUBLE:
            ss << GetDouble();
            break;
        case JSON_STRING:
            ss << '"' << EscapeString(GetString()) << '"';
            break;
        case JSON_ARRAY:
        {
            ss << '[';
            for (uint32 i = 0; i < _vec.size(); ++i)
            {
                if (i)
                    ss << ", ";
                _vec[i].Write(ss);
            }
            ss << ']';
            break;
        }
        case JSON_OBJECT:
        {
            ss << '{';
            for (uint64 i = 0; i < _mapOrder.size(); ++i)
            {
                if (i)
                    ss << ',';
                std::string key = _mapOrder[i];
                ss << '"' << key << "\":";
                _map[key].Write(ss);
            }
            ss << '}';
            break;
        }
        default:
            throw "Unknown type";
    }
}

std::string JSON::EscapeString(std::string str)
{
    static std::map<char, std::string> escapes = boost::assign::map_list_of
    ('"', "\\\"")
    ('\'', "\\'");
    /*('\\', "\\\\")
    ('/', "\\/")
    ('b', "\\b")
    ('f', "\\f")
    ('n', "\\n")
    ('r', "\\r")
    ('t', "\\t")
    ('u', "\\u");*/
    
    for (uint32 i = 0; i < str.length(); ++i)
    {
        if (escapes.count(str[i])) {
            std::string esc = escapes[str[i]];
            str.replace(i, 1, esc.c_str(), esc.length());
            i += esc.length()-1;
        }
    }
    
    return str;
}
