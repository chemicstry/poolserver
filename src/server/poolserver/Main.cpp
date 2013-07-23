#include <iostream>

#include "Server.h"
#include "Config.h"
#include "Log.h"
#include <boost/program_options.hpp>
#include <iostream>

bool InitConfig(int argc, char *argv[])
{
    // Containers
    boost::program_options::options_description descGeneric;
	boost::program_options::options_description descServer;
	boost::program_options::options_description descStratum;
	boost::program_options::options_description descLogging;
	boost::program_options::options_description cmdlineOptions;
	boost::program_options::options_description fileOptions;
    
    // Generic
    descGeneric.add_options()
		("version,v", "print version string")
		("help,h", "produce help message")
		("config,c", boost::program_options::value<std::string>()->default_value("settings.cfg"),"name of a file of a configuration.")
    ;
	
	// Server
    descServer.add_options()
        ("MinDiffTime", boost::program_options::value<uint32_t>()->default_value(100), "Minimum server diff time")
    ;
	
	// Stratum
    descStratum.add_options()
        ("StratumHost,sh", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "Stratum server host")
		("StratumPort,sp", boost::program_options::value<uint16_t>()->default_value(3333), "Stratum server port")
    ;
	
	// Logging
	descLogging.add_options()
        ("LogConsoleLevel", boost::program_options::value<uint32_t>()->default_value(LOG_LEVEL_INFO), "Console log level (0-None, 1-Error, 2-Warn, 3-Info, 4-Debug)")
		("LogConsoleDebugMask", boost::program_options::value<uint32_t>()->default_value(0), "Console log debug mask")
		("LogFilePath", boost::program_options::value<std::string>()->default_value("."), "File log path")
		("LogFileLevel", boost::program_options::value<uint32_t>()->default_value(LOG_LEVEL_WARN), "File log level (0-None, 1-Error, 2-Warn, 3-Info, 4-Debug)")
		("LogFileDebugMask", boost::program_options::value<uint32_t>()->default_value(0), "File log debug mask")
    ;
	
    cmdlineOptions.add(descGeneric).add(descServer).add(descStratum).add(descLogging);
    fileOptions.add(descServer).add(descStratum).add(descLogging);
    
    store(boost::program_options::command_line_parser(argc, argv).options(cmdlineOptions).run(), sConfig.vm);
    notify(sConfig.vm);
	
	if (sConfig.vm.count("help")) {
		std::cout << cmdlineOptions << std::endl;
		return false;
	}
    
    std::ifstream ifs(sConfig.Get<std::string>("LogFilePath").c_str());
	
	if (!ifs) {
        sLog.Error(LOG_GENERAL, "Failed opening config file: %s", sConfig.Get<std::string>("LogFilePath").c_str());
		return false;
	}
    
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
