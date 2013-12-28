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
#include <boost/thread/mutex.hpp>
#include <set>

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
            std::set<ClientPtr>::iterator it;
            for (it = _clients.begin(); it != _clients.end(); ++it)
                (*it)->Disconnect();
        }
        
        // Starts accepting connections
        void Start(tcp::endpoint endpoint);
        
        void SetBitcoinRPC(JSONRPC* bitcoinrpc)
        {
            _bitcoinrpc = bitcoinrpc;
        }
        
        // Sends message to all clients
        void SendToAll(JSON msg);
        
        // Returns new extranonce
        uint32 GetExtranonce()
        {
            return _extranonce++;
        }
        
        // Returns current work
        Bitcoin::BlockPtr GetWork()
        {
            boost::lock_guard<boost::mutex> guard(_mtx_workupdate);
            return _currentWork;
        }
        
        // Resets work for all clients
        void ResetWork();
        
        // Submits block to bitcoind
        bool SubmitBlock(Bitcoin::Block block);
        
        // Disconnects client
        void Disconnect(ClientPtr client)
        {
            client->Disconnect();
            _clients.erase(client);
        }
        
    private:
        void _StartAccept()
        {
            ClientPtr client = ClientPtr(new Client(this, _io_service));
            
            _acceptor.async_accept(client->GetSocket(), boost::bind(&Server::_OnAccept, this, client, asio::placeholders::error));
        }
        
        void _OnAccept(ClientPtr client, const boost::system::error_code& error)
        {
            if (!error) {
                sLog.Debug(LOG_STRATUM, "New stratum client accepted");
                client->Start();
                _clients.insert(client);
            } else {
                sLog.Debug(LOG_STRATUM, "Failed to accept stratum client");
            }
            
            _StartAccept();
        }
        
        void _UpdateWork(bool reset)
        {
            JSON response = _bitcoinrpc->Query("getblocktemplate");
            
            Bitcoin::BlockPtr block = Bitcoin::BlockPtr(new Bitcoin::Block());
            
            block->version = response["version"].GetInt();
            block->prevBlockHash = Util::Reverse(Util::ASCIIToBin(response["previousblockhash"].GetString()));
            block->time = response["curtime"].GetInt();
            // Set bits
            ByteBuffer bitbuf(Util::Reverse(Util::ASCIIToBin(response["bits"].GetString())));
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
            _mtx_workupdate.lock();
            _currentWork = block;
            _mtx_workupdate.unlock();
            
            // Requests all clients to reset work
            if (reset)
                ResetWork();
        }
        
        void _CheckBlocksTimer()
        {
            _CheckBlocks();
            _blockCheckTimer.expires_from_now(boost::posix_time::milliseconds(sConfig.Get<uint32>("StratumBlockCheckTime")));
            _blockCheckTimer.async_wait(boost::bind(&Server::_CheckBlocksTimer, this));
        }
        
        void _CheckBlocks()
        {
            // Might be called twice from timer and when block is found
            boost::lock_guard<boost::mutex> guard(_mtx_checkblock);
            
            sLog.Debug(LOG_STRATUM, "Clients: %u", _clients.size());
            
            JSON response = _bitcoinrpc->Query("getinfo");
            uint32 curBlock = response["blocks"].GetInt();
            
            if (curBlock > _blockHeight) {
                sLog.Debug(LOG_STRATUM, "New block on network! Height: %u", curBlock);
                _blockHeight = curBlock;
                _UpdateWork(true);
            }
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
        std::set<ClientPtr> _clients;
        asio::io_service& _io_service;
        tcp::acceptor _acceptor;
        
        // Mutexes
        boost::mutex _mtx_checkblock;
        boost::mutex _mtx_workupdate;
        
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
