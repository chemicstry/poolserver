#ifndef STRATUM_SERVER_H
#define STRATUM_SERVER_H

#include <boost/cstdint.hpp>
#include <string>

namespace Stratum
{
    class Server
    {
    public:
        Server(std::string ip, uint32_t port);
        ~Server();

    private:

    };
}

#endif
