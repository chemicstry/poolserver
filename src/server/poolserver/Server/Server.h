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
    Server();
    ~Server();

    Stratum::Server* stratumServer;

    boost::chrono::steady_clock::time_point diffStart;
    bool running;
    uint64_t serverLoops;

    int Run();
    void Update(uint32_t);
    
    bool InitDatabase();
};

#endif
