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
        Server(asio::io_service& io_service) : _io_service(io_service), _acceptor(io_service), _blockCheckTimer(io_service), _blockHeight(0), _extranonce(0)
        {
            _pubkey = Util::ASCIIToBin(sConfig.Get<std::string>("MiningAddress"));
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
        
        uint32 GetExtranonce()
        {
            return _extranonce++;
        }
        
        Bitcoin::BlockPtr GetWork()
        {
            return _currentWork;
        }
        
    private:
        void _StartAccept()
        {
            Client* client = new Client(this, _io_service);
            
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
        
        void _UpdateWork(bool reset)
        {
            JSON response = _bitcoinrpc->Query("getblocktemplate");
            
            Bitcoin::BlockPtr block = Bitcoin::BlockPtr(new Bitcoin::Block());
            
            block->version = response["version"].GetInt();
            block->prevBlockHash = Util::ASCIIToBin(response["previousblockhash"].GetString());
            block->time = response["curtime"].GetInt();
            // Set bits
            ByteBuffer bitbuf(Util::ASCIIToBin(response["bits"].GetString()));
            bitbuf >> block->bits;
            
            // Add coinbase tx
            block->tx.push_back(CreateCoinbaseTX(_blockHeight, _pubkey, response["coinbasevalue"].GetInt()));
            
            // Add other transactions
            JSON trans = response["transactions"];
            for (uint64 i = 0; i < trans.Size(); ++i) {
                ByteBuffer txbuf(Util::ASCIIToBin(trans[i]["data"].GetString()));
                Bitcoin::Transaction tx;
                txbuf >> tx;
                block->tx.push_back(tx);
            }
            
            // Genrate merkle tree
            block->BuildMerkleTree();
            
            // Set current work
            _currentWork = block;
        }
        
        void _CheckBlocks()
        {
            sLog.Debug(LOG_STRATUM, "Checking for new blocks...");
            
            JSON response = _bitcoinrpc->Query("getinfo");
            uint32 curBlock = response["blocks"].GetInt();
            
            if (curBlock > _blockHeight) {
                sLog.Debug(LOG_STRATUM, "New block on network! Height: %u", curBlock);
                _blockHeight = curBlock;
                _UpdateWork(true);
            }
            
            _blockCheckTimer.expires_from_now(boost::posix_time::milliseconds(sConfig.Get<uint32>("StratumBlockCheckTime")));
            _blockCheckTimer.async_wait(boost::bind(&Server::_CheckBlocks, this));
        }
        
        Bitcoin::Transaction CreateCoinbaseTX(uint32 blockHeight, BinaryData pubkey, int64 value)
        {
            // Extranonce placeholder
            BinaryData extranonce_ph(8, 0);
            ByteBuffer scriptsig;
            scriptsig << _blockHeight << extranonce_ph;
            
            Bitcoin::OutPoint outpoint;
            outpoint.hash.resize(32, 0);
            outpoint.n = 0xFFFFFFFF;
            
            Bitcoin::TxIn txin;
            txin.prevout = outpoint;
            txin.script = scriptsig.Binary();
            txin.n = 0;
            
            Bitcoin::TxOut txout;
            txout.value = value;
            txout.scriptPubKey = Bitcoin::Script(pubkey) + Bitcoin::OP_CHECKSIG;
            
            Bitcoin::Transaction tx;
            tx.version = 1;
            tx.in.push_back(txin);
            tx.out.push_back(txout);
            tx.lockTime = 0;
            
            return tx;
        }
    private:
        // Network
        std::list<Client*> _clients;
        asio::io_service& _io_service;
        tcp::acceptor _acceptor;
        
        // RPC
        JSONRPC* _bitcoinrpc;
        
        // Bitcoin info
        BinaryData _pubkey;
        asio::deadline_timer _blockCheckTimer;
        uint32 _blockHeight;
        
        // Work
        Bitcoin::BlockPtr _currentWork;
        uint32 _extranonce;
    };
}

#endif
