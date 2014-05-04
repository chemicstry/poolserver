#ifndef JSON_H_
#define JSON_H_

#include "Common.h"
#include "Log.h"
#include "Util.h"
#include "Exception.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/variant.hpp>
#include <string>
#include <utility>
#include <sstream>
#include <vector>
#include <map>

enum JSONValueType
{
    JSON_NULL       = 0,
    JSON_ARRAY      = 1,
    JSON_OBJECT     = 2,
    JSON_BOOL       = 3,
    JSON_INTEGER    = 4,
    JSON_DOUBLE     = 5,
    JSON_STRING     = 6
};

typedef boost::variant<bool, int64, double, std::string> JSONValue;

class JSONException: public Exception
{
public:
    JSONException(const char *text): Exception(text) {}
    JSONException(std::string text): Exception(text) {}
};

class JSON
{
public:
    static JSON FromString(std::string jsonstring);
    
    JSON(JSONValueType type = JSON_NULL) : _type(type)
    {
    }
    
    ~JSON()
    {
    }
    
    // Arrays
    JSON& operator[] (uint32 index)
    {
        if (index >= _vec.size())
            throw JSONException(Util::FS("Index %u out of range. Vector size: %u", index, _vec.size()));
        
        return _vec[index];
    }
    
    template<typename T>
    void Add(T val)
    {
        if (_type != JSON_ARRAY) {
            if (_type != JSON_NULL)
                throw JSONException("Node is not an array type");
            else
                _type = JSON_ARRAY;
        }
        
        JSON node;
        node = val;
        _vec.push_back(node);
    }
    
    void AddNull()
    {
        if (_type != JSON_ARRAY) {
            if (_type != JSON_NULL)
                throw JSONException("Node is not an array type");
            else
                _type = JSON_ARRAY;
        }
        
        JSON node;
        _vec.push_back(node);
    }
    
    // Objects
    JSON& operator[] (std::string key)
    {
        if (_type != JSON_OBJECT) {
            if (_type != JSON_NULL)
                throw JSONException("Node is not an object type");
            else
                _type = JSON_OBJECT;
        }
        
        // Keep insertion order
        if (!_map.count(key))
            _mapOrder.push_back(key);
        
        // std::map[] automatically creates key if it does not exist
        return _map[key];
    }
    
    template<typename T>
    void Set(std::string key, T val)
    {
        if (_type != JSON_OBJECT) {
            if (_type != JSON_NULL)
                throw JSONException("Node is not an object type");
            else
                _type = JSON_OBJECT;
        }
        
        // Keep insertion order
        if (!_map.count(key))
            _mapOrder.push_back(key);
        
        // std::map[] automatically creates key if it does not exist
        _map[key] = val;
    }
    
    // Primitive types
    bool GetBool()
    {
        if (_type != JSON_BOOL)
            throw JSONException("Node is not a bool type");
        
        return boost::get<bool>(_val);
    }
    
    void operator= (bool val)
    {
        _type = JSON_BOOL;
        _val = val;
    }
    
    int64 GetInt()
    {
        if (_type != JSON_INTEGER)
            throw JSONException("Node is not an int type");
        
        return boost::get<int64>(_val);
    }
    
    void operator= (int64 val)
    {
        _type = JSON_INTEGER;
        _val = val;
    }
    
    double GetDouble()
    {
        if (_type != JSON_DOUBLE)
            throw JSONException("Node is not a double type");
        
        return boost::get<double>(_val);
    }
    
    void operator= (double val)
    {
        _type = JSON_DOUBLE;
        _val = val;
    }
    
    std::string GetString()
    {
        if (_type != JSON_STRING)
            throw JSONException("Node is not a string type");
        
        return boost::get<std::string>(_val);
    }
    
    void operator= (std::string val)
    {
        _type = JSON_STRING;
        _val = val;
    }
    
    // Fix for static strings
    void operator= (const char* val)
    {
        _type = JSON_STRING;
        _val = std::string(val);
    }
    
    // Generic function
    bool Empty()
    {
        if (_type == JSON_NULL)
            return true;
        else
            return false;
    }
    
    uint32 Size()
    {
        if (_type == JSON_ARRAY)
            return _vec.size();
        else if (_type == JSON_OBJECT)
            return _map.size();
        else
            return 0;
    }
    
    JSONValueType GetType()
    {
        return _type;
    }
    
    void SetType(JSONValueType type)
    {
        _type = type;
    }
    
    // Used for writting
    std::string EscapeString(std::string);
    std::string ToString();
    void Write(std::stringstream& ss);
    void SetRaw(std::string val);
    void AddRaw(std::string val);
    
    std::string hackfix;
private:
    // Vector is used to store arrays
    std::vector<JSON> _vec;
    // Map is used to store objects
    std::map<std::string, JSON> _map;
    // Map insertion order
    std::vector<std::string> _mapOrder;
    // Boost variant stores primitive types
    JSONValue _val;
    
    JSONValueType _type;
};

template<>
inline void JSON::Add(JSON& node)
{
    if (_type != JSON_ARRAY) {
        if (_type != JSON_NULL)
            throw JSONException("Node is not an array type");
        else
            _type = JSON_ARRAY;
    }
    
    _vec.push_back(node);
}

#endif
