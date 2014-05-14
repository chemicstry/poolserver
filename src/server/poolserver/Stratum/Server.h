#ifndef STRATUM_SERVER_H_
#define STRATUM_SERVER_H_

#include "Common.h"
#include "Client.h"
#include "Config.h"
#include "Log.h"
#include "JSON.h"
#include "JSONRPC.h"
#include "Bitcoin.h"
#include "Util.h"
#include "ByteBuffer.h"
#include "NetworkMgr.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <set>
#include <vector>

#define MAX_PACKET 4096

using namespace boost;
using namespace boost::asio::ip;

namespace Stratum
{
    struct ListenInfo
    {
        std::string Host;
        uint16 Port;
    };
    
    struct BanInfo
    {
        uint32 ip;
        uint64 timestamp;
    };
    
    // Used for sorting std::set
    struct ClientPtrCMP
    {
        bool operator() (const ClientPtr& a, const ClientPtr& b)
        {
            return a->GetID() < b->GetID();
        }
    };
    
    class Server
    {
    public:
        Server(asio::io_service& io_service) : _io_service(io_service), _acceptor(io_service), _extranonce(0), _clientId(0)
        {
            // Subscribe for block updates
            NetworkMgr::Instance()->BlockNotifyBind(boost::bind(&Server::BlockNotify, this, _1, _2));
        }
        
        ~Server()
        {
            std::set<ClientPtr>::iterator it;
            for (it = _clients.begin(); it != _clients.end(); ++it)
                (*it)->Disconnect();
        }
        
        // Starts accepting connections
        void Start(tcp::endpoint endpoint);
        
        // Sends message to all clients
        void SendToAll(JSON msg);
        
        // Returns new extranonce
        uint32 GetExtranonce()
        {
            boost::lock_guard<boost::mutex> guard(_mtxExtranonce);
            return _extranonce++;
        }
        
        // Returns current work
        Bitcoin::BlockPtr GetWork()
        {
            boost::lock_guard<boost::mutex> guard(_mtxCurrentWork);
            return _currentWork;
        }
        
        // Block template update event
        void BlockNotify(Bitcoin::BlockPtr block, bool newBlock)
        {
            sLog.Debug(LOG_STRATUM, "Received block template update");
            _mtxCurrentWork.lock();
            _currentWork = block;
            _mtxCurrentWork.unlock();
            SendBlockTmpl(newBlock);
        }
        
        // Resets work for all clients
        void SendBlockTmpl(bool resetWork);
        
        // Submits block to bitcoind
        bool SubmitBlock(Bitcoin::Block block);
        
        // Disconnects client
        void Disconnect(ClientPtr client)
        {
            _mtxClients.lock();
            _clients.erase(client);
            _mtxClients.unlock();
            
            client->CloseSocket();
            sLog.Info(LOG_STRATUM, "Stratum client disconnected from %s. Total clients: %u", asio::ip::address_v4(client->GetIP()).to_string().c_str(), _clients.size());
        }
        
        void Ban(uint32 ip, uint64 time)
        {
            BanInfo ban;
            ban.ip = ip;
            ban.timestamp = Util::Date() + time;
            
            _mtxBans.lock();
            _bans.push_back(ban);
            _mtxBans.unlock();
        }
        
        bool IsBanned(uint32 ip)
        {
            boost::lock_guard<boost::mutex> guard(_mtxBans);
            for (int i = 0; i < _bans.size(); ++i) {
                if (_bans[i].ip == ip) {
                    if (_bans[i].timestamp > Util::Date())
                        return true;
                }
            }
            
            return false;
        }
        
    private:
        void _StartAccept()
        {
            ClientPtr client = ClientPtr(new Client(this, _io_service, _clientId++));
            
            _acceptor.async_accept(client->GetSocket(), boost::bind(&Server::_OnAccept, this, client, asio::placeholders::error));
        }
        
        void _OnAccept(ClientPtr client, const boost::system::error_code& error)
        {
            if (!error) {
                if (client->Start()) {
                    _mtxClients.lock();
                    _clients.insert(client);
                    _mtxClients.unlock();
                    
                    sLog.Info(LOG_STRATUM, "New stratum client accepted from %s. Total clients: %u", asio::ip::address_v4(client->GetIP()).to_string().c_str(), _clients.size());
                }
            } else {
                sLog.Debug(LOG_STRATUM, "Failed to accept stratum client");
            }
            
            _StartAccept();
        }

    private:
        // Network
        asio::io_service& _io_service;
        tcp::acceptor _acceptor;
        
        // Clients
        std::set<ClientPtr, ClientPtrCMP> _clients;
        boost::mutex _mtxClients;
        uint64 _clientId;
        
        // Bands
        std::vector<BanInfo> _bans;
        boost::mutex _mtxBans;
        
        // Work
        Bitcoin::BlockPtr _currentWork;
        boost::mutex _mtxCurrentWork;
        uint32 _extranonce;
        boost::mutex _mtxExtranonce;
    };
}

#endif
