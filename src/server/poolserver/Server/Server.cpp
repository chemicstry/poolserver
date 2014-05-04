#include "Server.h"
#include "Config.h"
#include "JSONRPC.h"
#include "Log.h"
#include "Stratum/Server.h"
#include "ServerDatabaseEnv.h"
#include "Crypto.h"
#include "Bitcoin.h"
#include "DataMgr.h"
#include "NetworkMgr.h"
#include "Exception.h"

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <algorithm>

Server::Server(asio::io_service& io) : serverLoops(0), io_service(io), uploadtimer(io)
{
}

Server::~Server()
{
    //delete stratumServer;
}

void AsyncQueryCallback(MySQL::QueryResult result)
{
}



void Server::UploadShares(const boost::system::error_code& /*e*/)
{
    sDataMgr.Upload();
    uploadtimer.expires_from_now(boost::posix_time::seconds(3)); //repeat rate here
    uploadtimer.async_wait(boost::bind(&Server::UploadShares, this, boost::asio::placeholders::error));
}

int Server::Run()
{
    sLog.Info(LOG_SERVER, "Server is starting...");
    
    InitDatabase();
    
    /*std::vector<Share> shares;

    sLog.Info(LOG_SERVER, "Loading shares...");
    
    MySQL::QueryResult result = sDatabase.Query("SELECT MIN(`id`), MAX(`id`) FROM `shares`");
    MySQL::Field* fields = result->FetchRow();
    uint32 min = fields[0].Get<uint32>();
    uint32 max = fields[1].Get<uint32>();
    sLog.Info(LOG_SERVER, "Min: %u Max: %u", min, max);
    
    MySQL::PreparedStatement* stmt = sDatabase.GetPreparedStatement(STMT_QUERY_SHARES);
    for (uint32 i = min; i <= max; i += 500000)
    {
        stmt->SetUInt32(0, i);
        MySQL::QueryResult result2 = sDatabase.Query(stmt);
        while (MySQL::Field* fields2 = result2->FetchRow()) {
            shares.push_back(Share(fields2[0].Get<uint64>(), fields2[1].Get<uint32>(), 2981, fields2[2].Get<uint32>()));
        }
        sLog.Info(LOG_SERVER, "Shares: %u", shares.size());
    }
    
    sLog.Info(LOG_SERVER, "Loaded %u shares", shares.size());*/
    
    /*std::vector<byte> test = Util::ASCIIToBin("4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
    sLog.Info(LOG_SERVER, "Hash: %s", Util::BinToASCII(Crypto::SHA256D(test)).c_str());
    sLog.Info(LOG_SERVER, "RevHash: %s", Util::BinToASCII(Crypto::SHA256D(Util::Reverse(test))).c_str());
    
    Bitcoin::Block block;
    
    ByteBuffer buf(Util::ASCIIToBin("01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000"));
    Bitcoin::Transaction trans;
    buf >> trans;
    sLog.Info(LOG_SERVER, "Version: %u", trans.version);
    sLog.Info(LOG_SERVER, "Inputs: %u", trans.in.size());
    sLog.Info(LOG_SERVER, "PrevOut: %s", Util::BinToASCII(trans.in[0].prevout.hash).c_str());
    sLog.Info(LOG_SERVER, "PrevOutn: %u", trans.in[0].prevout.n);
    sLog.Info(LOG_SERVER, "ScriptSig: %s", Util::BinToASCII(trans.in[0].script.script).c_str());
    sLog.Info(LOG_SERVER, "Inn: %u", trans.in[0].n);
    sLog.Info(LOG_SERVER, "Outputs: %u", trans.out.size());
    sLog.Info(LOG_SERVER, "Value: %i", trans.out[0].value);
    sLog.Info(LOG_SERVER, "PubSig: %s", Util::BinToASCII(trans.out[0].scriptPubKey.script).c_str());
    sLog.Info(LOG_SERVER, "LockTime: %u", trans.lockTime);
    block.tx.resize(1);
    block.tx[0] = trans;
    block.BuildMerkleTree();
    sLog.Info(LOG_SERVER, "Hash: %s", Util::BinToASCII(block.merkleRootHash).c_str());*/
    
    /*ByteBuffer buf(Util::ASCIIToBin("01000000010c432f4fb3e871a8bda638350b3d5c698cf431db8d6031b53e3fb5159e59d4a90000000000ffffffff0100f2052a010000001976a9143744841e13b90b4aca16fe793a7f88da3a23cc7188ac00000000"));
    
    Bitcoin::Transaction trans;
    buf >> trans;
    
    ByteBuffer buf2;
    buf2 << trans;
    
    sLog.Info(LOG_SERVER, "Trans: %s", Util::BinToASCII(buf2.vec).c_str());*/
    
    
    
    NetworkMgr::Initialize(io_service);
    
    std::vector<std::string> btcrpc = sConfig.Get<std::vector<std::string> >("BitcoinRPC");
    for (int i = 0; i < btcrpc.size(); ++i) {
        std::vector<std::string> params = Util::Explode(btcrpc[i], ";");
        
        if (params.size() != 4)
            throw Exception("Invalid Bitcoin RPC parameters");
        
        JSONRPCConnectionInfo coninfo;
        coninfo.Host = params[0];
        coninfo.Port = params[1];
        coninfo.User = params[2];
        coninfo.Pass = params[3];
        
        NetworkMgr::Instance()->Connect(coninfo);
    }
    
    Stratum::Server srv(io_service);
    
    // Start stratum server
    tcp::endpoint endpoint(tcp::v4(), sConfig.Get<uint16>("StratumPort"));
    srv.Start(endpoint);
    
    // Start timer
    uploadtimer.expires_from_now(boost::posix_time::seconds(3)); //repeat rate here
    uploadtimer.async_wait(boost::bind(&Server::UploadShares, this,  boost::asio::placeholders::error));
    
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
            boost::this_thread::sleep(boost::posix_time::milliseconds(sleepDuration));
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

