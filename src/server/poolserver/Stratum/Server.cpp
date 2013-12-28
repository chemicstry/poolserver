#include "Server.h"

namespace Stratum
{
    void Server::Start(tcp::endpoint endpoint)
    {
        // Start block checking timer
        _CheckBlocksTimer();
        
        _acceptor.open(endpoint.protocol());
        _acceptor.set_option(tcp::acceptor::reuse_address(true));
        _acceptor.bind(endpoint);
        _acceptor.listen();
        
        _StartAccept();
        
        sLog.Debug(LOG_STRATUM, "Stratum server started");
    }
    
    void Server::SendToAll(JSON msg)
    {
        std::set<ClientPtr>::iterator it;
        for (it = _clients.begin(); it != _clients.end(); ++it)
            _io_service.post(boost::bind(&Client::SendMessage, (*it), msg));
    }
    
    void Server::ResetWork()
    {
        std::set<ClientPtr>::iterator it;
        for (it = _clients.begin(); it != _clients.end(); ++it)
            _io_service.post(boost::bind(&Client::SendJob, (*it), true));
    }
    
    bool Server::SubmitBlock(Bitcoin::Block block)
    {
        // Serialize block
        ByteBuffer blockbuf;
        blockbuf << block;
        
        try {
            JSON params;
            params.Add(Util::BinToASCII(blockbuf.Binary()));
            JSON response = _bitcoinrpc->Query("submitblock", params);
            
            if (response["result"].GetType() == JSON_NULL) {
                sLog.Info(LOG_STRATUM, "Block accepted! YAY!");
                _io_service.post(boost::bind(&Server::_CheckBlocks, this));
                return true;
            } else {
                sLog.Info(LOG_STRATUM, "Block rejected! Booooo");
            }
        } catch (...) {
            return false;
        }
    }
}
