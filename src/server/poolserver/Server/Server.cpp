#include "Server.h"
#include "Config.h"
#include "JSONRPC.h"
#include "Log.h"
#include "Stratum/Server.h"
#include "ServerDatabaseEnv.h"
#include "Crypto.h"
#include "Bitcoin.h"

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <algorithm>

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
    while (MySQL::Field* fields = result->FetchRow()) {
        sLog.Info(LOG_SERVER, "Row: %i %s", fields[0].GetUInt32(), fields[1].GetString().c_str());
    }
}

int Server::Run()
{
    sLog.Info(LOG_SERVER, "Server is starting...");
    
    //InitDatabase();
    
    Bitcoin::OutPoint outpoint;
    outpoint.hash = Util::ASCIIToBin("6DBDDB085B1D8AF75184F0BC01FAD58D1266E9B63B50881990E4B40D6AEE3629");
    outpoint.n = 0;
    
    Bitcoin::TxOut txout;
    txout.value = 5000000;
    txout.scriptPubKey = Bitcoin::Script(Util::ASCIIToBin("76A9141AA0CD1CBEA6E7458A7ABAD512A9D9EA1AFB225E88AC"));
    
    ByteBuffer buf;
    buf << txout;
    sLog.Info(LOG_SERVER, "Serialized: %s", Util::BinToASCII(buf.vec).c_str());
    
    // Main io service
    asio::io_service io_service;
    
    Stratum::Server srv(io_service);
    
    // Init Bitcoin RPC
    JSONRPCConnectionInfo coninfo;
    coninfo.Host = "84.240.15.208";
    coninfo.Port = "9902";
    coninfo.User = "ppcuser";
    coninfo.Pass = "DYAL6bC4RUHksL6ikdx7";
    
    JSONRPC* bitcoinrpc = new JSONRPC();
    bitcoinrpc->Connect(coninfo);
    srv.SetBitcoinRPC(bitcoinrpc);
    
    // Start stratum server
    tcp::endpoint endpoint(tcp::v4(), sConfig.Get<uint16>("StratumPort"));
    srv.Start(endpoint);
    
    io_service.run();
    
    
    //sDatabase.Execute("INSERT INTO `test_table` VALUES ('999', 'sync', '1.1')");
    //sDatabase.ExecuteAsync("INSERT INTO `test_table` VALUES ('999', 'sync', '1.1')");
    
    /*MySQL::PreparedStatement* stmt = sDatabase.GetPreparedStatement(STMT_INSERT_SHIT);
    stmt->SetUInt32(0, 10);
    stmt->SetString(1, "hello");
    stmt->SetFloat(2, 5.987);
    sDatabase.ExecuteAsync(stmt);*/
    
    //MySQL::PreparedStatement* stmt = sDatabase.GetPreparedStatement(STMT_QUERY_TEST_TABLE);
    //MySQL::QueryResult result = sDatabase.Query(stmt);
    
    
    //sDatabase.QueryAsync("SELECT * FROM `test_table`", &AsyncQueryCallback);
    //MySQL::QueryResult result = sDatabase.Query("SELECT * FROM `test_table`");
    /*if (result) {
        sLog.Info(LOG_SERVER, "Metadata: F: %u R: %u", result->GetFieldCount(), result->GetRowCount());
        while (MySQL::Field* fields = result->FetchRow()) {
            sLog.Info(LOG_SERVER, "Row: %i %s", fields[0].GetUInt32(), fields[1].GetString().c_str());
        }
    } else
        sLog.Info(LOG_SERVER, "Empty result");*/
    
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

