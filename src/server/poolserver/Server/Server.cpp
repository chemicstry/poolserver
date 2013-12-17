#include "Server.h"
#include "Config.h"
#include "Log.h"
#include "Stratum/Server.h"
#include "ServerDatabaseEnv.h"

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

Server::Server() : serverLoops(0)
{
}

Server::~Server()
{
    //delete stratumServer;
}

void AsyncQueryCallback(MySQL::QueryResult result)
{
    sLog.Info(LOG_SERVER, "Metadata: F: %u R: %u", result->GetFieldCount(), result->GetRowCount());
    while (result->NextRow()) {
        MySQL::Field* fields = result->Fetch();
        sLog.Info(LOG_SERVER, "Row: %i %s", fields[0].GetUInt32(), 
        fields[1].GetString().c_str());
    }
}

int Server::Run()
{
    sLog.Info(LOG_SERVER, "Server is starting...");
    
    InitDatabase();
    
    //sDatabase.Execute("INSERT INTO `test_table` VALUES ('999', 'sync', '1.1')");
    //sDatabase.ExecuteAsync("INSERT INTO `test_table` VALUES ('999', 'sync', '1.1')");
    
    sDatabase.QueryAsync("SELECT * FROM `test_table`", &AsyncQueryCallback);
    MySQL::QueryResult result = sDatabase.Query("SELECT * FROM `test_table`");
    sLog.Info(LOG_SERVER, "Metadata: F: %u R: %u", result->GetFieldCount(), result->GetRowCount());
    while (result->NextRow()) {
        MySQL::Field* fields = result->Fetch();
        sLog.Info(LOG_SERVER, "Row: %i %s", fields[0].GetUInt32(), 
        fields[1].GetString().c_str());
    }
    
    // Start stratum server
    sLog.Info(LOG_SERVER, "Starting stratum");
    //stratumServer = new Stratum::Server(Config::GetString("STRATUM_IP"), Config::GetInt("STRATUM_PORT"));
    
    // Init loop vars
    uint32_t sleepDuration = 0;
    int exitcode = 0;
    running = true;

    // Init diff
    uint32_t minDiffTime = sConfig.Get<uint32_t>("MinDiffTime");
    diffStart = boost::chrono::steady_clock::now();
    
    sLog.Info(LOG_SERVER, "Server is running!");

    while (running)
    {
        // Calc time diff
        boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
        uint32_t diff = boost::chrono::duration_cast<boost::chrono::milliseconds>(now - diffStart).count();
        diffStart = now;

        // Update
        Update(diff);

        // Mercy for CPU
        if (diff < minDiffTime+sleepDuration) {
            sleepDuration = minDiffTime - diff + sleepDuration;
            boost::this_thread::sleep_for(boost::chrono::milliseconds(sleepDuration));
        } else
            sleepDuration = 0;

        ++serverLoops;

        if (serverLoops > 50)
            running = false;
        //std::cout << "Diff: " << diff << ", Loop: " << serverLoops << std::endl;
    }
    
    sLog.Info(LOG_SERVER, "Server is stopping...");
    
    sDatabase.Close();

    return exitcode;
}

void Server::Update(uint32_t diff)
{

}

bool Server::InitDatabase()
{
    if (boost::iequals(sConfig.Get<std::string>("DatabaseDriver"), "mysql")) {
        MySQL::ConnectionInfo connInfo;
        connInfo.Host = sConfig.Get<std::string>("MySQLHost");
        connInfo.Port = sConfig.Get<uint16_t>("MySQLPort");
        connInfo.User = sConfig.Get<std::string>("MySQLUser");
        connInfo.Pass = sConfig.Get<std::string>("MySQLPass");
        connInfo.DB = sConfig.Get<std::string>("MySQLDatabase");
        return sDatabase.Open(connInfo, sConfig.Get<uint16_t>("MySQLSyncThreads"), sConfig.Get<uint16_t>("MySQLAsyncThreads"));
    } else {
        sLog.Error(LOG_SERVER, "Database Driver '%s' not found.", sConfig.Get<std::string>("DatabaseDriver").c_str());
        return false;
    }
}

