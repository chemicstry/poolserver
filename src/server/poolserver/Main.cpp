#include "Server.h"
#include "Config.h"
#include "Log.h"
#include "Util.h"
#include "JSON.h"
#include "JSONRPC.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

bool InitConfig(int argc, char *argv[])
{
    // Containers
    boost::program_options::options_description descGeneric("Generic Options");
    boost::program_options::options_description descServer("Server Configuration");
    boost::program_options::options_description descStratum("Stratum Configuration");
    boost::program_options::options_description descLogging("Logging Configuration");
    boost::program_options::options_description descDatabase("Database Configuration");
    #ifdef WITH_MYSQL
        boost::program_options::options_description descMySQL("MySQL Configuration");
    #endif
    boost::program_options::options_description cmdlineOptions;
    boost::program_options::options_description fileOptions;
    
    // Generic
    descGeneric.add_options()
        ("version,v", "Print version string")
        ("help,h", "Produce help message")
        ("config,c", boost::program_options::value<std::string>()->default_value("../etc/poolserver.cfg"),"Path to configuration file")
    ;
    
    // Server
    descServer.add_options()
        ("ServerThreads", boost::program_options::value<uint32_t>()->default_value(2), "How many threads to use")
        ("MiningAddress", boost::program_options::value<std::string>(), "Address to send coins to")
        ("BitcoinRPC", boost::program_options::value<std::vector<std::string> >()->multitoken(), "Bitcoin RPC login credentials")
        ("ShareUploadMinCount", boost::program_options::value<uint32>()->default_value(2), "Minimum share count to upload to database")
        ("ShareUploadBulkCount", boost::program_options::value<uint32>()->default_value(50), "How many shares to upload in one query")
        ("ShareUploadInterval", boost::program_options::value<uint32>()->default_value(3), "How often to upload shares")
    ;
    
    // Stratum
    descStratum.add_options()
        ("StratumHost,sh", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Bind IP for stratum")
        ("StratumRedirectHost", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Where to redirect getwork requests")
        ("StratumPort,sp", boost::program_options::value<uint16_t>()->default_value(3333), "Stratum server port")
        ("StratumBlockCheckTime", boost::program_options::value<uint32>()->default_value(2000), "Time between block checks in ms")
        ("RetargetInterval", boost::program_options::value<uint32>()->default_value(20), "Time between difficulty checks in seconds")
        ("RetargetSharesThreshold", boost::program_options::value<uint32>()->default_value(20), "Number of shares in retarget interval to trigger a difficulty check")
        ("RetargetTimeBuffer", boost::program_options::value<uint32>()->default_value(60*5), "Buffer of shares to keep (in seconds)")
        ("RetargetTimePerShare", boost::program_options::value<double>()->default_value(4), "Target in seconds between shares")
        ("RetargetVariance", boost::program_options::value<uint32>()->default_value(40), "Maximum allowed variance in percent before difficulty change")
        ("RetargetStartingDiff", boost::program_options::value<uint32>()->default_value(2), "Difficulty at which new miner starts")
        ("RetargetMinDiff", boost::program_options::value<uint32>()->default_value(1), "Minimum difficulty (also starting difficulty)")
        ("RetargetMaxDiff", boost::program_options::value<uint32>()->default_value(1000000), "Maximum difficulty we can reach")
    ;
    
    // Logging
    descLogging.add_options()
        ("LogConsoleLevel", boost::program_options::value<uint32_t>()->default_value(LOG_LEVEL_INFO), "Console log level (0-None, 1-Error, 2-Warn, 3-Info, 4-Debug)")
        ("LogConsoleDebugMask", boost::program_options::value<uint32_t>()->default_value(0), "Console log debug mask")
        ("LogFilePath", boost::program_options::value<std::string>()->default_value("../etc"), "File log path")
        ("LogFileLevel", boost::program_options::value<uint32_t>()->default_value(LOG_LEVEL_WARN), "File log level (0-None, 1-Error, 2-Warn, 3-Info, 4-Debug)")
        ("LogFileDebugMask", boost::program_options::value<uint32_t>()->default_value(0), "File log debug mask")
    ;
    
    // Database
    #ifdef WITH_MYSQL
    descDatabase.add_options()
        ("DatabaseDriver", boost::program_options::value<std::string>()->default_value("mysql"), "Database Driver")
    ;
    descMySQL.add_options()
        ("MySQLHost", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "MySQL Host")
        ("MySQLPort", boost::program_options::value<uint16_t>()->default_value(3306), "MySQL Port")
        ("MySQLUser", boost::program_options::value<std::string>()->default_value("root"), "MySQL User")
        ("MySQLPass", boost::program_options::value<std::string>()->default_value(""), "MySQL Password")
        ("MySQLDatabase", boost::program_options::value<std::string>()->default_value("poolserver"), "MySQL Database")
        ("MySQLSyncThreads", boost::program_options::value<uint16_t>()->default_value(2), "MySQL Sync Threads to Create")
        ("MySQLAsyncThreads", boost::program_options::value<uint16_t>()->default_value(2), "MySQL Async Threads to Create")
    ;
    descDatabase.add(descMySQL);
    #endif
    
    cmdlineOptions.add(descGeneric).add(descServer).add(descStratum).add(descLogging).add(descDatabase);
    fileOptions.add(descServer).add(descStratum).add(descLogging).add(descDatabase);
    
    store(boost::program_options::command_line_parser(argc, argv).options(cmdlineOptions).run(), sConfig.vm);
    notify(sConfig.vm);
    
    if (sConfig.vm.count("help")) {
        std::cout << cmdlineOptions << std::endl;
        return false;
    }
    
    std::ifstream ifs(sConfig.Get<std::string>("config").c_str());
    
    if (!ifs.is_open()) {
        sLog.Error(LOG_GENERAL, "Failed opening config file: %s", sConfig.Get<std::string>("config").c_str());
        return true;
    }
    
    sLog.Info(LOG_GENERAL, "Using config file: %s", sConfig.Get<std::string>("config").c_str());
    
    store(parse_config_file(ifs, fileOptions), sConfig.vm);
    notify(sConfig.vm);
    
    return true;
}

int main(int argc, char *argv[])
{
    if (!InitConfig(argc, argv))
        return 0;
    
    sLog.OpenLogFile(sConfig.Get<std::string>("LogFilePath"));
    sLog.Info(LOG_GENERAL, "LogFile Started: %s", sLog.logFileLoc.c_str());
    
    Server* server = new Server();
    int exitcode = server->Run();
    delete server;

    return exitcode;
}
