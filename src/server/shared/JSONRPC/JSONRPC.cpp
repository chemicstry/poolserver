#include "JSONRPC.h"

#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include "Util.h"
#include "Log.h"

bool JSONRPC::Connect(JSONRPCConnectionInfo connInfo)
{
    _connInfo = connInfo;
    
    if (_connInfo.B64Auth.empty())
        _connInfo.B64Auth = Util::ToBase64(_connInfo.User + ":" + _connInfo.Pass, false);
    
    sLog.Debug(LOG_JSONRPC, "JSONRPC::Connect(): B64Auth: %s", _connInfo.B64Auth.c_str());

    boost::asio::ip::tcp::resolver resolver(_ios);
    boost::asio::ip::tcp::resolver::query q(_connInfo.Host, _connInfo.Port);
    boost::asio::ip::tcp::resolver::iterator epi = resolver.resolve(q);
    boost::asio::ip::tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    
    while (error && epi != end)
    {
        _sock.close();
        _sock.connect(*epi, error);
    }
    
    if (error)
    {
        sLog.Error(LOG_JSONRPC, "JSONRPC::Connect(): Error connecting to '%s': %s", _connInfo.Host.c_str(), boost::system::system_error(error).what());
        return false;
    }
    
    _ep = *epi;
    
    _sock.close();

    return true;
}

JSON JSONRPC::Query(std::string method, JSON params)
{
    JSON request(true);
    request.Set("jsonrpc", "1.0");
    request.Set("id", 1);
    request.Set("method", method);
    if (params.Size() > 0)
        request.Set("params", params);
    std::string jsonstring = request.ToString();
    
    sLog.Debug(LOG_JSONRPC, "JSONRPC::Query(): JSONString: %s", jsonstring.c_str());
    
    boost::system::error_code error;
    
    _sock.close();
    _sock.connect(_ep, error);
    
    if (error)
    {
        sLog.Error(LOG_JSONRPC, "JSONRPC::Query(): Error connecting to '%s': %s", _connInfo.Host.c_str(), boost::system::system_error(error).what());
        return NULL;
    }

    boost::asio::streambuf request_buf;
    std::ostream request_info(&request_buf);
    request_info << "POST / HTTP/1.1\n";
    request_info << "Host: 127.0.0.1\n";
    request_info << "Content-Type: application/json-rpc\n";
    request_info << "Authorization: Basic " << _connInfo.B64Auth << "\n";
    request_info << "Content-Length: " << jsonstring.size() << "\n\n";
    request_info << jsonstring;

    boost::asio::write(_sock, request_buf);

    boost::asio::streambuf response;
    boost::asio::read_until(_sock, response, "\n");

    std::istream response_stream(&response);
    
    std::string http_version;
    uint32_t status_code;
    std::string status_message;
    
    response_stream >> http_version >> status_code;
    std::getline(response_stream, status_message);

    
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        sLog.Error(LOG_JSONRPC, "JSONRPC::Query(): Malformed HTTP Response");
        return NULL;
    }
    
    if (status_code != 200)
    {
        sLog.Error(LOG_JSONRPC, "JSONRPC::Query(): Returned status code: %u", status_code);
        return NULL;
    }
    
    std::vector<std::string> headers;
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
        headers.push_back(header);
    
    std::string jsonresponse = "";
    
    if (response.size() > 0) {
        std::ostringstream oss;
        oss << &response;
        jsonresponse += oss.str();
    }

    // Read until EOF, writing data to output as we go.
    while(boost::asio::read(_sock, response, boost::asio::transfer_at_least(1), error)){
        std::ostringstream oss;
        oss << &response;
        jsonresponse += oss.str();
    }
    
    _sock.close();
    
    sLog.Debug(LOG_JSONRPC, "JSONRPC::Query(): JSON Response: %s", jsonresponse.c_str());
    
    return JSON::FromString(jsonresponse);
}