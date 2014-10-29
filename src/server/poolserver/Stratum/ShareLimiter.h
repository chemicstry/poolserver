#ifndef SHARELIMITER_H_
#define SHARELIMITER_H_

#include "Common.h"
#include "Util.h"

#include <deque>

// 2^32 * 10^-6
#define MEGAHASHCONST 4294.967296

namespace Stratum
{
    class Client;
    
    struct ShareLimiterRecord
    {
        ShareLimiterRecord(uint64 adiff, uint64 atime) : diff(adiff), time(atime) {}
        uint64 diff;
        uint64 time;
    };
    
    class ShareLimiter
    {
    public:
        ShareLimiter(Client* client) : _client(client), _totalShares(0), _totalBadShares(0)
        {
            // Minus to prevent crash when interval is zero
            _startTime = Util::Date()-10;
            _lastRetargetTime = _startTime;
        }
        
        bool Submit(uint64 diff);
        
        void LogBad()
        {
            ++_totalBadShares;
        }
        
    private:
        std::deque<ShareLimiterRecord> _shares;
        Client* _client;
        uint64 _lastRetargetTime;
        uint64 _lastRetargetShares;
        uint64 _startTime;
        
        uint64 _totalShares;
        uint64 _totalBadShares;
    };
}

#endif
