#ifndef CONFIG_H_
#define CONFIG_H_

#include <cassert>
#include <cstring>
#include <boost/program_options.hpp>
#include <boost/cstdint.hpp>
#include "Log.h"

class Config
{
public:
    Config();
    ~Config();
    
    // Reading
    template<class T>
    T Get(std::string key)
    {
        T tmp;
        
        try {
            tmp = vm[key].as<T>();
        } catch(std::exception& e) {
            sLog.Error(LOG_GENERAL, "Failed to get config value for key '%s'", key.c_str());
        }
        
        return tmp;
    }
    
    // Containers
    boost::program_options::variables_map vm;
};

extern Config sConfig;

#endif
