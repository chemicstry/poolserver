#ifndef STRATUM_CLIENT_H_
#define STRATUM_CLIENT_H_

#include "Common.h"
#include "Log.h"
#include "JSON.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define MAX_PACKET 2048

using namespace boost;
using namespace boost::asio::ip;

namespace Stratum
{
    class Client
    {
    public:
        Client(asio::io_service& io_service) : _socket(io_service)
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
        
        void SendMessage(JSON msg)
        {
            std::string data = msg.ToString();
            _socket.send(boost::asio::buffer(data.c_str(), data.length()));
        }
        
        void OnMessage(JSON msg)
        {
            std::string method = msg.Get<std::string>("method");
            sLog.Debug(LOG_SERVER, "Method: %s", method.c_str());
        }
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
        asio::streambuf _recvBuffer;
        tcp::socket _socket;
    };
}

#endif
