#include "Log.h"
#include "Config.h"
#include "Util.h"

Log sLog;

Log::Log(): logfile(NULL)
{
}

Log::~Log()
{
    if (logfile)
        logfile.close();
}

void Log::Error(LogType type, const char * str, ...)
{
    va_list ap;
    va_start(ap, str);

    char text[MAX_MSG_LEN];
    vsnprintf(text, MAX_MSG_LEN, str, ap);
    Write(LOG_LEVEL_ERROR, type, std::string(text));

    va_end(ap);
}

void Log::Warn(LogType type, const char * str, ...)
{
    va_list ap;
    va_start(ap, str);

    char text[MAX_MSG_LEN];
    vsnprintf(text, MAX_MSG_LEN, str, ap);
    Write(LOG_LEVEL_WARN, type, std::string(text));

    va_end(ap);
}

void Log::Info(LogType type, const char * str, ...)
{
    va_list ap;
    va_start(ap, str);

    char text[MAX_MSG_LEN];
    vsnprintf(text, MAX_MSG_LEN, str, ap);
    Write(LOG_LEVEL_INFO, type, std::string(text));

    va_end(ap);
}

void Log::Debug(LogType type, const char * str, ...)
{
    va_list ap;
    va_start(ap, str);

    char text[MAX_MSG_LEN];
    vsnprintf(text, MAX_MSG_LEN, str, ap);
    Write(LOG_LEVEL_DEBUG, type, std::string(text));

    va_end(ap);
}

void Log::OpenLogFile(std::string path)
{
    if (!logfile) {
        logFileLoc = path + "/server-" + Util::Date("%Y%m%d-%H%M%S") + ".log";
        logfile.open(logFileLoc.c_str());
    }
}

void Log::Write(LogLevel level, LogType type, std::string msg)
{
    boost::lock_guard<boost::mutex> lock(_mutex);
    
    switch(level)
    {
        case LOG_LEVEL_ERROR:
            if (sConfig.Get<uint32_t>("LogConsoleLevel") >= level)
                std::cout << "[ERROR] " << msg << std::endl;
            if (sConfig.Get<uint32_t>("LogFileLevel") >= level)
                AppendFile("[ERROR] " + msg);
            break;
        case LOG_LEVEL_WARN:
            if (sConfig.Get<uint32_t>("LogConsoleLevel") >= level)
                std::cout << "[WARN] " << msg << std::endl;
            if (sConfig.Get<uint32_t>("LogFileLevel") >= level)
                AppendFile("[WARN] " + msg);
            break;
        case LOG_LEVEL_INFO:
            if (sConfig.Get<uint32_t>("LogConsoleLevel") >= level)
                std::cout << "[INFO] " << msg << std::endl;
            if (sConfig.Get<uint32_t>("LogFileLevel") >= level)
                AppendFile("[INFO] " + msg);
            break;
        case LOG_LEVEL_DEBUG:
            if (sConfig.Get<uint32_t>("LogConsoleLevel") >= level) {
                uint32_t debugmask = sConfig.Get<uint32_t>("LogConsoleDebugMask");
                if (debugmask & uint32_t(pow(2, type)))
                    std::cout << "[DEBUG] " << msg << std::endl;
            }
            if (sConfig.Get<uint32_t>("LogFileLevel") >= level) {
                uint32_t debugmask = sConfig.Get<uint32_t>("LogFileDebugMask");
                if (debugmask & uint32_t(pow(2, type)))
                    AppendFile("[DEBUG] " + msg);
            }
            break;
    }
}

void Log::AppendFile(std::string msg)
{
    if (!logfile)
        return;
    
    logfile << msg << std::endl;
}
