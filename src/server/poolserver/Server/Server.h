#ifndef SERVER_H_
#define SERVER_H_

#include "Stratum/Server.h"

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/chrono.hpp>

#define SERVER_MIN_DIFF 100

class Server
{
public:
    Server(asio::io_service& io);
    ~Server();

    Stratum::Server* stratumServer;

    void UploadShares(const boost::system::error_code& e);
    boost::chrono::steady_clock::time_point diffStart;
    bool running;
    uint64_t serverLoops;

    int Run();
    void Update(uint32_t);
    
    bool InitDatabase();
    
    asio::io_service& io_service;
};

#endif
