#ifndef SERVER_H_
#define SERVER_H_

#include "Stratum/Server.h"

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>

#define SERVER_MIN_DIFF 100

class Server
{
public:
    Server();
    ~Server();
    
    int Run();
    bool InitDatabase();
    
    void WorkerThread(boost::shared_ptr<asio::io_service> io_service);

private:
    // Protocol servers
    Stratum::Server* _stratumServer;
    
    // Main io service
    boost::shared_ptr<asio::io_service> _io_service;
    
    // Main thread group
    boost::thread_group _workerThreads;
};

#endif
