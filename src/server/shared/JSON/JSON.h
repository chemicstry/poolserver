#ifndef JSON_H_
#define JSON_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/cstdint.hpp>
#include <string>
#include <utility>

class JSON
{
public:
    static JSON FromString(std::string jsonstring);
    
    JSON(bool base = true) : _base(base), pt(NULL)
    {
        if (base)
            pt = new boost::property_tree::ptree();
    }
    
    ~JSON()
    {
        //if (_base && pt)
        //    delete pt;
    }
    
    template<class T>
    T Get(std::string key)
    {
        return pt->get<T>(key);
    }
    
    JSON operator[] (std::string key)
    {
        JSON json(false);
        json.pt = &pt->get_child(key);
        return json;
    }
    
    JSON operator[] (int index)
    {
        JSON json(false);
        boost::property_tree::ptree::iterator it = pt->begin();
        for (int i = 0; i < index; ++i)
            ++it;
        boost::property_tree::ptree::value_type& kv = *it;
        json.pt = &kv.second;
        return json;
    }
    
    template<class T>
    inline void Set(std::string key, T value)
    {
        pt->put<T>(key, value);
    }
    
    template<class T>
    inline void Add(T value)
    {
        pt->push_back(std::make_pair("", value));
    }
    
    uint64_t Size()
    {
        return pt->size();
    }
    
    std::string ToString();
    
    boost::property_tree::ptree* pt;
    bool _base;
};

template<>
inline void JSON::Set<JSON>(std::string key, JSON value)
{
    pt->put_child(key, *value.pt);
}

template<>
inline void JSON::Add<JSON>(JSON value)
{
    pt->push_back(std::make_pair("", *value.pt));
}

#endif
