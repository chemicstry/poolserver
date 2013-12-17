#include "JSON.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <boost/foreach.hpp>

JSON JSON::FromString(std::string jsonstring)
{
    JSON json;
    json.pt = new boost::property_tree::ptree();
    std::stringstream ss;
    ss.str(jsonstring);
    boost::property_tree::json_parser::read_json(ss, *json.pt);
    return json;
}

std::string JSON::ToString()
{
    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, *pt);
    return ss.str();
}
