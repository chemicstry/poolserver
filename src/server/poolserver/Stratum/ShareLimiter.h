#ifndef SHARELIMITER_H_
#define SHARELIMITER_H_

#include "Common.h"
#include "Util.h"

#include <deque>

namespace Stratum
{
    class Client;
    
    class ShareLimiter
    {
    public:
        ShareLimiter(Client* client) : _client(client), _totalShares(0), _totalBadShares(0)
        {
            _startTime = Util::Date();
            _lastRetarget = _startTime;
        }
        
        bool Submit();
        
        void LogBad()
        {
            ++_totalBadShares;
        }
        
    private:
        std::deque<uint64> _shares;
        Client* _client;
        uint64 _lastRetarget;
        uint64 _startTime;
        
        uint64 _totalShares;
        uint64 _totalBadShares;
    };
}

#endif
