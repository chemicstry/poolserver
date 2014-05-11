#include "NetworkMgr.h"
#include "Config.h"

NetworkMgr* NetworkMgr::singleton = 0;

NetworkMgr::NetworkMgr(asio::io_service& io_service) : _io_service(io_service), _blockNotifyTimer(io_service), _blockCheckTimer(io_service), _blockHeight(0)
{
    BlockCheckTimerStart();
    BlockNotifyTimerStart();
}

NetworkMgr::~NetworkMgr()
{
    for (int i = 0; i < _cons.size(); ++i)
        delete _cons[i];
}

// Bitcoin daemon connection
void NetworkMgr::Connect(JSONRPCConnectionInfo coninfo)
{
    JSONRPC* bitcoinrpc = new JSONRPC();
    bitcoinrpc->Connect(coninfo);
    
    JSON response = bitcoinrpc->Query("getinfo");
    
    if (response["error"].GetType() != JSON_NULL)
        throw Exception(Util::FS("Failed to get response from bitcoin rpc: %s", response["error"].GetString().c_str()));
    
    sLog.Info(LOG_SERVER, "Connected to Bitcoin RPC at %s:%s", coninfo.Host.c_str(), coninfo.Port.c_str());
    
    _cons.push_back(bitcoinrpc);
    
    // Fetch block height on first connection
    if (_cons.size() == 1)
        BlockCheck();
}

// Get new block template
void NetworkMgr::UpdateBlockTemplate()
{
    // Might be called by block notify timer and block check timer so we need a lock
    boost::lock_guard<boost::mutex> guard(_mtxBlockTmpl);
    
    for (int i = 0; i < _cons.size(); ++i)
    {
        try {
            JSON response = _cons[i]->Query("getblocktemplate");
            
            // New blocks may not appear on all daemons the same time
            if (response["height"].GetInt() < _blockHeight)
                continue;
            
            Bitcoin::BlockPtr block = Bitcoin::BlockPtr(new Bitcoin::Block());
            
            block->version = response["version"].GetInt();
            block->prevBlockHash = Util::Reverse(Util::ASCIIToBin(response["previousblockhash"].GetString()));
            block->time = response["curtime"].GetInt();
            // Set bits
            ByteBuffer bitbuf(Util::Reverse(Util::ASCIIToBin(response["bits"].GetString())));
            bitbuf >> block->bits;
            
            // Add coinbase tx
            BinaryData pubkey = Util::ASCIIToBin(sConfig.Get<std::string>("MiningAddress"));
            block->tx.push_back(Bitcoin::CreateCoinbaseTX(_blockHeight, pubkey, response["coinbasevalue"].GetInt()));
            
            // Add other transactions
            JSON trans = response["transactions"];
            for (uint64 tidx = 0; tidx < trans.Size(); ++tidx) {
                ByteBuffer txbuf(Util::ASCIIToBin(trans[tidx]["data"].GetString()));
                Bitcoin::Transaction tx;
                txbuf >> tx;
                block->tx.push_back(tx);
            }
            
            // Genrate merkle tree
            block->BuildMerkleTree();
            
            // Set
            _curBlockTmpl = block;
            
            sLog.Info(LOG_SERVER, "Fetched block template from rpc #%u", i);
            
            // Break from loop
            break;
        } catch (std::exception& e) {
            sLog.Error(LOG_SERVER, "Failed to fetch block template from daemon #%u: %s", i, e.what());
        }
    }
}

// Submit new block
bool NetworkMgr::SubmitBlock(Bitcoin::Block block)
{
    // Serialize block
    ByteBuffer blockbuf;
    blockbuf << block;
    
    for (int i = 0; i < _cons.size(); ++i)
    {
        try {
            JSON params;
            params.Add(Util::BinToASCII(blockbuf.Binary()));
            JSON response = _cons[i]->Query("submitblock", params);
            
            if (response["result"].GetType() == JSON_NULL) {
                sLog.Info(LOG_SERVER, "Block accepted! YAY!");
                BlockCheck();
                return true;
            } else {
                sLog.Error(LOG_SERVER, "Block rejected: %s", response["error"].GetString().c_str());
            }
        } catch (std::exception& e) {
            sLog.Error(LOG_SERVER, "Exception caught while submiting block: %s", e.what());
        }
    }
    
    return false;
}

// Check for new blocks
void NetworkMgr::BlockCheck()
{
    // Might be called twice from timer and when block is found
    boost::lock_guard<boost::mutex> guard(_mtxBlockCheck);
    
    for (int i = 0; i < _cons.size(); ++i)
    {
        try {
            JSON response = _cons[i]->Query("getinfo");
            uint32 curBlock = response["blocks"].GetInt();
            
            if (curBlock > _blockHeight) {
                sLog.Debug(LOG_SERVER, "New block on network! Height: %u", curBlock);
                _blockHeight = curBlock;
                
                // Update block template
                UpdateBlockTemplate();
                
                // Send notifications (and reset work)
                BlockNotifySend(true);
            }
        } catch (std::exception& e) {
            sLog.Error(LOG_SERVER, "Failed to fetch block height from daemon #%u: %s", i, e.what());
        }
    }
}

void NetworkMgr::BlockCheckTimerStart()
{
    _blockCheckTimer.expires_from_now(boost::posix_time::seconds(2));
    _blockCheckTimer.async_wait(boost::bind(&NetworkMgr::BlockCheckTimerExpired, this,  boost::asio::placeholders::error));
}

void NetworkMgr::BlockCheckTimerExpired(const boost::system::error_code& /*e*/)
{
    BlockCheck();
    BlockCheckTimerStart();
}

// Bind for receiving block notifications
void NetworkMgr::BlockNotifyBind(FBlockNotify f)
{
    _blockNotifyBinds.push_back(f);
    // Send block template
    f(_curBlockTmpl, true);
}

// Block notify timer
void NetworkMgr::BlockNotifyTimerStart()
{
    _blockNotifyTimer.expires_from_now(boost::posix_time::seconds(30));
    _blockNotifyTimer.async_wait(boost::bind(&NetworkMgr::BlockNotifyTimerExpired, this,  boost::asio::placeholders::error));
}

void NetworkMgr::BlockNotifyTimerExpired(const boost::system::error_code& /*e*/)
{
    sLog.Debug(LOG_SERVER, "Sending block template update");
    
    UpdateBlockTemplate();
    BlockNotifySend();
    
    BlockNotifyTimerStart();
}

// Send block notification to all subscribers
void NetworkMgr::BlockNotifySend(bool newBlock)
{
    for (int i = 0; i < _blockNotifyBinds.size(); ++i)
        _io_service.post(boost::bind(_blockNotifyBinds[i], _curBlockTmpl, newBlock));
}
