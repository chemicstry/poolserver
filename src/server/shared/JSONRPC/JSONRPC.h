#ifndef JSONRPC_H_
#define JSONRPC_H_

// Some of the JSON RPC code was written by https://github.com/bytemaster/cpp_bitcoin_rpc

#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <string>

#include "JSON.h"
#include "Exception.h"

struct JSONRPCConnectionInfo
{
    std::string Host;
    std::string Port;
    std::string User;
    std::string Pass;
    std::string B64Auth;
};

class JSONRPCException: public Exception
{
public:
    JSONRPCException(const char *text): Exception(text) {}
    JSONRPCException(std::string text): Exception(text) {}
};

class JSONRPC
{
public:
    JSONRPC(): _sock(_ios) {}
    bool Connect(JSONRPCConnectionInfo conninfo);
    JSON Query(std::string method, JSON params = JSON::FromString("[]"));

private:
    JSONRPCConnectionInfo _connInfo;
    boost::asio::io_service _ios;
    boost::asio::ip::tcp::socket _sock;
    boost::asio::ip::tcp::endpoint _ep;
};

#endif
