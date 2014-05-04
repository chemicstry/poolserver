#ifndef SHARELIMITER_H_
#define SHARELIMITER_H_

#include "Common.h"
#include "Util.h"

#include <deque>

#define RETARGET_INTERVAL 60
#define RETARGET_TIME_BUFFER 60*5
#define RETARGET_SHARES_PER_MIN 15
#define RETARGET_VARIANCE 40

namespace Stratum
{
    class Client;
    
    class ShareLimiterRecord
    {
    public:
        ShareLimiterRecord(uint64 atime, uint64 adiff) : time(atime), diff(adiff) {}
        uint64 time;
        uint64 diff;
    };
    
    class ShareLimiter
    {
    public:
        ShareLimiter(Client* client) : _client(client), _lastRetarget(0)
        {
            _startTime = Util::Date();
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
