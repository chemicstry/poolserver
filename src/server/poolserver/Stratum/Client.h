#ifndef STRATUM_CLIENT_H_
#define STRATUM_CLIENT_H_

#include "Common.h"
#include "Config.h"
#include "Log.h"
#include "JSON.h"
#include "Server.h"
#include "Job.h"
#include "ShareLimiter.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <set>

#define MAX_PACKET 2048

using namespace boost;
using namespace boost::asio::ip;

namespace Stratum
{
    class Server;
    
    class Client : public boost::enable_shared_from_this<Client>
    {
    public:
        Client(Server* server, asio::io_service& io_service, uint64 id) : _server(server), _socket(io_service), _id(id), _subscribed(false), _jobid(0), _shareLimiter(this)
        {
            _diff = sConfig.Get<uint32>("StratumMinDifficulty");
            _minDiff = _diff;
        }
        
        tcp::socket& GetSocket()
        {
            return _socket;
        }
        
        // Start client up!
        void Start();
        
        void StartRead()
        {
            // Read until newline
            boost::asio::async_read_until(
                _socket,
                _recvBuffer,
                '\n',
                boost::bind(&Client::_OnReceive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
        }
        
        void SendJob(bool clean);
        
        void SendMessage(JSON msg)
        {
            std::string data = msg.ToString();
            data += '\n';
            sLog.Debug(LOG_SERVER, "Sending: %s", data.c_str());
            boost::asio::async_write(
                _socket,
                boost::asio::buffer(data.c_str(), data.length()),
                boost::bind(&Client::_OnSend, this, boost::asio::placeholders::error));
        }
        
        void OnMiningSubmit(JSON msg);
        void OnMiningSubscribe(JSON msg);
        void OnMiningAuthorize(JSON msg);
        
        void OnMessage(JSON msg)
        {
            std::string method = msg["method"].GetString();
            sLog.Debug(LOG_SERVER, "Decoded: %s", msg.ToString().c_str());
            
            if (method.compare("mining.submit") == 0)
                OnMiningSubmit(msg);
            else if (method.compare("mining.subscribe") == 0)
                OnMiningSubscribe(msg);
            else if (method.compare("mining.authorize") == 0)
                OnMiningAuthorize(msg);
            else
                sLog.Error(LOG_SERVER, "Method '%s' not found.", method.c_str());
        }
        
        // Gets new job from the server
        Job GetJob();
        
        // Worker difficulty
        uint64 GetDifficulty()
        {
            return _diff;
        }
        void SetDifficulty(uint64 diff, bool resendJob = false)
        {
            _diff = diff;
            
            // Send difficulty update
            JSON params;
            params.Add(int64(_diff));
            
            JSON msg;
            msg["id"];
            msg["params"] = params;
            msg["method"] = "mining.set_difficulty";
            
            SendMessage(msg);
            
            if (resendJob)
                SendJob(false);
        }
        
        // Client ID
        uint64 GetID()
        {
            return _id;
        }
        
        void Ban(uint32 time);
        void Disconnect();
        
        void CloseSocket()
        {
            _socket.close();
        }
    public:
        void _OnReceive(const boost::system::error_code& error, size_t bytes_transferred);
        void _OnSend(const boost::system::error_code& error);
        
    private:
        // Networking
        asio::streambuf _recvBuffer;
        tcp::socket _socket;
        uint32 _ip;
        uint64 _id;
        
        // Pointer to server
        Stratum::Server* _server;
        
        // Authorization
        std::set<std::string> _workers;
        
        // Jobs
        bool _subscribed;
        uint32 _extranonce;
        std::map<uint64, Job> _jobs;
        uint32 _jobid;
        
        // Share limiting
        uint64 _diff;
        uint64 _minDiff;
        ShareLimiter _shareLimiter;
    };
    
    typedef boost::shared_ptr<Client> ClientPtr;
}

#endif
