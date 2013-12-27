#ifndef STRATUM_CLIENT_H_
#define STRATUM_CLIENT_H_

#include "Common.h"
#include "Log.h"
#include "JSON.h"
#include "Server.h"
#include "Job.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define MAX_PACKET 2048

using namespace boost;
using namespace boost::asio::ip;

namespace Stratum
{
    class Server;
    
    class Client
    {
    public:
        Client(Server* server, asio::io_service& io_service) : _server(server), _socket(io_service), _subscribed(false), _jobid(0)
        {
        }
        
        tcp::socket& GetSocket()
        {
            return _socket;
        }
        
        void Start()
        {
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
            _socket.send(boost::asio::buffer(data.c_str(), data.length()));
        }
        
        void OnMiningSubscribe(JSON msg);
        
        void OnMiningAuthorize(JSON msg)
        {
            std::string username = msg["result"][0].GetString();
            JSON response;
            response["id"] = msg["id"].GetInt();
            response["error"];
            response["result"] = true;
            SendMessage(response);
        }
        
        void OnMessage(JSON msg)
        {
            std::string method = msg["method"].GetString();
            sLog.Debug(LOG_SERVER, "Method: %s", method.c_str());
            
            if (method.compare("mining.subscribe") == 0)
                OnMiningSubscribe(msg);
            else if (method.compare("mining.authorize") == 0)
                OnMiningAuthorize(msg);
            else
                sLog.Error(LOG_SERVER, "Method '%s' not found.", method.c_str());
        }
        
        Job GetJob();
    public:
        void _OnReceive(const boost::system::error_code& error, size_t bytes_transferred)
        {
            std::istream is(&_recvBuffer);
            std::stringstream iss;
            iss << is.rdbuf();
            sLog.Debug(LOG_SERVER, "Received: %s", iss.str().c_str());
            OnMessage(JSON::FromString(iss.str()));
        }
    private:
        // Networking
        asio::streambuf _recvBuffer;
        tcp::socket _socket;
        
        // Pointer to server
        Stratum::Server* _server;
        
        // Authorization
        std::vector<std::string> _workers;
        
        // Jobs
        bool _subscribed;
        uint32 _extranonce;
        std::map<uint64, Job> _jobs;
        uint64 _jobid;
    };
}

#endif
