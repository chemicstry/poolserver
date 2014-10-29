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

using namespace boost;
using namespace boost::asio::ip;

namespace Stratum
{
    class Server;
    
    class Client : public boost::enable_shared_from_this<Client>
    {
    public:
        Client(Server* server, asio::io_service& io_service, uint64 id) : _io_service(io_service), _server(server), _socket(io_service), _ioStrand(io_service), _id(id), _subscribed(false), _jobid(0), _shareLimiter(this)
        {
            _diff = sConfig.Get<uint32>("RetargetStartingDiff");
            _minDiff = sConfig.Get<uint32>("RetargetMinDiff");
        }
        
        ~Client()
        {
            CleanJobs();
            sLog.Info(LOG_STRATUM, "%u: I'm going out! Cya!", _ip);
        }
        
        tcp::socket& GetSocket()
        {
            return _socket;
        }
        
        // Start client up!
        bool Start();
        
        void StartRead()
        {
            // Read until newline
            asio::async_read(
                _socket,
                _recvBuffer,
                asio::transfer_at_least(1),
                _ioStrand.wrap(boost::bind(&Client::_OnReceive, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred)));
        }
        
        void SendJob(bool clean);
        
        void SendMessage(JSON msg)
        {
            boost::unique_lock<boost::mutex> lock(_mtxWrite);
            
            std::string data = msg.ToString();
            data += '\n';
            sLog.Debug(LOG_SERVER, "Sending: %s", data.c_str());
            boost::asio::async_write(
                _socket,
                boost::asio::buffer(data.c_str(), data.length()),
                _ioStrand.wrap(boost::bind(&Client::_OnSend, shared_from_this(), boost::asio::placeholders::error)));
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
                sLog.Debug(LOG_STRATUM, "Method '%s' not found.", method.c_str());
        }
        
        // Gets new job from the server
        Job* GetJob();
        
        void CleanJobs()
        {
            for (std::map<uint64, Job*>::iterator it = _jobs.begin(); it != _jobs.end(); ++it)
                delete it->second;
            _jobs.clear();
        }
        
        // Worker difficulty
        uint64 GetDifficulty()
        {
            return _diff;
        }
        void SetDifficulty(uint64 diff, bool resendJob = false)
        {
            if (diff < _minDiff)
                diff = _minDiff;
            
            if (diff == _diff)
                return;
            
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
        
        uint32 GetIP()
        {
            return _ip;
        }
        
        void Ban(uint32 time);
        void Disconnect();
        
        void RedirectGetwork()
        {
            sLog.Info(LOG_STRATUM, "Sending redirect to stratum for client %u", _ip);
            std::string redirect(Util::FS("HTTP/1.1 200 OK\r\nX-Stratum: stratum+tcp://%s:%u\r\nConnection: Close\r\nContent-Length: 41\r\n\r\n{\"error\": null, \"result\": false, \"id\": 0}\n", sConfig.Get<std::string>("StratumRedirectHost").c_str(), sConfig.Get<uint16>("StratumPort")));
            boost::asio::async_write(
                _socket,
                boost::asio::buffer(redirect.c_str(), redirect.length()),
                _ioStrand.wrap(boost::bind(&Client::_OnSend, shared_from_this(), boost::asio::placeholders::error)));
        }
        
        void CloseSocket()
        {
            boost::system::error_code ec;
            _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            _socket.close(ec);
        }
    public:
        void _OnReceive(const boost::system::error_code& error, size_t bytes_transferred);
        void _OnSend(const boost::system::error_code& error);
        
    private:
        // ASIO
        asio::io_service& _io_service;
        
        // Networking
        asio::streambuf _recvBuffer;
        std::string _recvMessage;
        tcp::socket _socket;
        asio::strand _ioStrand;
        boost::mutex _mtxWrite;
        uint32 _ip;
        uint64 _id;
        
        // Pointer to server
        Stratum::Server* _server;
        
        // Authorization
        std::set<std::string> _workers;
        
        // Jobs
        bool _subscribed;
        uint32 _extranonce;
        std::map<uint64, Job*> _jobs;
        uint32 _jobid;
        
        // Share limiting
        uint64 _diff;
        uint64 _minDiff;
        ShareLimiter _shareLimiter;
    };
    
    typedef boost::shared_ptr<Client> ClientPtr;
}

#endif
