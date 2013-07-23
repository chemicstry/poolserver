#ifndef STRATUM_SERVER_H_
#define STRATUM_SERVER_H_

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
