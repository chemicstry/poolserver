#ifndef SHARELIMITER_H_
#define SHARELIMITER_H_

#include "Common.h"
#include "Util.h"

#include <deque>

#define RETARGET_INTERVAL 20
#define RETARGET_TIME_BUFFER 60*5
#define RETARGET_SHARES_PER_MIN 15
#define RETARGET_VARIANCE 40
#define RETARGET_MAXDIFF 1000000

namespace Stratum
{
    class Client;
    
    class ShareLimiter
    {
    public:
        ShareLimiter(Client* client) : _client(client)
        {
            _startTime = Util::Date();
            _lastRetarget = _startTime;
        }
        
        bool Submit();
        
    private:
        std::deque<uint64> _shares;
        Client* _client;
        uint64 _lastRetarget;
        uint64 _startTime;
    };
}

#endif
