#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdarg>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>

#define MAX_MSG_LEN 32*1024
#define ATTR_PRINTF(F, V) __attribute__ ((format (printf, F, V)))

enum LogType
{
    LOG_GENERAL                         = 0,
    LOG_SERVER                          = 1,
    LOG_DATABASE                        = 2,
    LOG_JSON                            = 3,
    LOG_JSONRPC                         = 4,
    LOG_STRATUM                         = 5,
};

enum LogLevel
{
    LOG_LEVEL_NONE                      = 0,
    LOG_LEVEL_ERROR                     = 1,
    LOG_LEVEL_WARN                      = 2,
    LOG_LEVEL_INFO                      = 3,
    LOG_LEVEL_DEBUG                     = 4
};

class Log
{
public:
    Log();
    ~Log();
    
    void Error(LogType type, const char * str, ...) ATTR_PRINTF(3, 4);
    void Warn(LogType type, const char * str, ...) ATTR_PRINTF(3, 4);
    void Info(LogType type, const char * str, ...) ATTR_PRINTF(3, 4);
    void Debug(LogType type, const char * str, ...) ATTR_PRINTF(3, 4);
    
    void OpenLogFile(std::string filename);
    std::string logFileLoc;
    
private:
    void Write(LogLevel level, LogType type, std::string msg);
    void AppendFile(std::string);
    
    std::string logfileloc;
    std::ofstream logfile;
    
    boost::mutex _mutex;
};

extern Log sLog;

#endif
