#ifndef STRATUM_SERVER_H_
#define STRATUM_SERVER_H_

#include "Common.h"
#include "Client.h"
#include "Config.h"
#include "Log.h"
#include "JSON.h"
#include "JSONRPC.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <list>

using namespace boost;
using namespace boost::asio::ip;

namespace Stratum
{
    struct ListenInfo
    {
        std::string Host;
        uint16 Port;
    };
    
    class Server
    {
    public:
        Server(asio::io_service& io_service) : _io_service(io_service), _acceptor(io_service), _blockCheckTimer(io_service), _blockHeight(0)
        {
        }
        
        ~Server()
        {
            std::list<Client*>::iterator it;
            for (it = _clients.begin(); it != _clients.end(); ++it)
                delete *it;
        }
        
        void Start(tcp::endpoint endpoint)
        {
            _CheckBlocks();
            
            _acceptor.open(endpoint.protocol());
            _acceptor.set_option(tcp::acceptor::reuse_address(true));
            _acceptor.bind(endpoint);
            _acceptor.listen();
            
            _StartAccept();
            
            sLog.Debug(LOG_STRATUM, "Stratum server started");
        }
        
        void SetBitcoinRPC(JSONRPC* bitcoinrpc)
        {
            _bitcoinrpc = bitcoinrpc;
        }
        
        void SendToAll(JSON msg)
        {
            std::list<Client*>::iterator it;
            for (it = _clients.begin(); it != _clients.end(); ++it)
                (*it)->SendMessage(msg);
        }
        
    private:
        void _StartAccept()
        {
            Client* client = new Client(_io_service);
            
            _acceptor.async_accept(client->GetSocket(), boost::bind(&Server::_OnAccept, this, client, asio::placeholders::error));
        }
        
        void _OnAccept(Client* client, const boost::system::error_code& error)
        {
            if (!error) {
                sLog.Debug(LOG_STRATUM, "New stratum client accepted");
                client->Start();
                _clients.push_back(client);
            } else {
                sLog.Debug(LOG_STRATUM, "Failed to accept stratum client");
                delete client;
            }
            
            _StartAccept();
        }
        
        void _CheckBlocks()
        {
            sLog.Debug(LOG_STRATUM, "Checking for new blocks...");
            
            JSON response = _bitcoinrpc->Query("getinfo");
            uint32 curBlock = response.Get<uint32>("blocks");
            
            // Initializing server
            if (_blockHeight == 0) {
                _blockHeight = curBlock;
            } else if (curBlock > _blockHeight) {
                sLog.Debug(LOG_STRATUM, "New block on network! Height: %u", curBlock);
                _blockHeight = curBlock;
                // do some crazy stuff
            }
            
            _blockCheckTimer.expires_from_now(boost::posix_time::milliseconds(sConfig.Get<uint32>("StratumBlockCheckTime")));
            _blockCheckTimer.async_wait(boost::bind(&Server::_CheckBlocks, this));
        }
    private:
        // Network
        std::list<Client*> _clients;
        asio::io_service& _io_service;
        tcp::acceptor _acceptor;
        
        // RPC
        JSONRPC* _bitcoinrpc;
        
        // Bitcoin info
        asio::deadline_timer _blockCheckTimer;
        uint32 _blockHeight;
    };
}

#endif
