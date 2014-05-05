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

Server::Server()
{
    _io_service = boost::shared_ptr<asio::io_service>(new asio::io_service());
}

Server::~Server()
{
    delete _stratumServer;
    
}

int Server::Run()
{
    sLog.Info(LOG_SERVER, "Server is starting...");
    
    // Connect to database
    InitDatabase();
    
    // Initialize share storage
    DataMgr::Initialize(*_io_service);
    
    // Initialize bitcoin daemons
    NetworkMgr::Initialize(*_io_service);
    
    // Connect to bitcoin rpc
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
    
    // Init stratum
    _stratumServer = new Stratum::Server(*_io_service);
    
    // Start stratum server
    tcp::endpoint endpoint(tcp::v4(), sConfig.Get<uint16>("StratumPort"));
    _stratumServer->Start(endpoint);
    
    for (uint32 i = 0; i < sConfig.Get<uint32>("ServerThreads"); ++i) {
		_workerThreads.create_thread(boost::bind(&Server::WorkerThread, this, _io_service));
	}
    
    _workerThreads.join_all();
    
    sLog.Info(LOG_SERVER, "Server is stopping...");
    
    sDatabase.Close();

    return 0;
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

void Server::WorkerThread(boost::shared_ptr<asio::io_service> io_service)
{
    std::stringstream threadid;
    threadid << boost::this_thread::get_id();
    
	sLog.Info(LOG_SERVER, "Main server thread #%s started.", threadid.str().c_str());

	while (true)
	{
		try
		{
			boost::system::error_code ec;
			_io_service->run(ec);
			if (ec) {
				sLog.Error(LOG_SERVER, "IO error caught in main server thread #%s: %s.", threadid.str().c_str(), ec.message().c_str());
			}
			break;
		} catch(std::exception& e) {
			sLog.Error(LOG_SERVER, "Exception caught in main server thread #%s: %s.", threadid.str().c_str(), e.what());
		}
	}

	sLog.Info(LOG_SERVER, "Main server thread #%s stopped.", threadid.str().c_str());
}

