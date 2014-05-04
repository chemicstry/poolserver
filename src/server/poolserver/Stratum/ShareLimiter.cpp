#include "ShareLimiter.h"
#include "Server.h"
#include "Client.h"
#include "Log.h"

namespace Stratum
{
    // Returning false will stop any further share verifications (DoS prevention, etc)
    bool ShareLimiter::Submit()
    {
        ++_totalShares;
        
        uint64 curTime = Util::Date();
        uint64 sinceLast = curTime - _lastRetarget;
        
        _shares.push_back(curTime);
        
        if (sinceLast < RETARGET_INTERVAL)
            return true;
        
        _lastRetarget = curTime;
        
        // Check if miner is ok
        if (_totalShares > 20 && (double(_totalBadShares)/double(_totalShares)) > 0.8)
            _client->Ban(60);
        
        while (_shares.size() && (_shares.front() < curTime - RETARGET_TIME_BUFFER))
            _shares.pop_front();
        
        uint32 interval = std::min(curTime - _startTime, uint64(RETARGET_TIME_BUFFER));
        
        // Calculate shares/min
        double speed = double(_shares.size()*60) / double(interval);
        
        // Calculate difference from pool target in %
        double variance = speed / double(RETARGET_SHARES_PER_MIN);
        
        sLog.Info(LOG_SERVER, "Miner variance: %f speed: %f", variance, speed);
        
        // Check if we need to retarget
        if (variance*100 < RETARGET_VARIANCE)
            return true;
        
        uint64 newDiff = double(_client->GetDifficulty()) * variance;
        
        if (newDiff < 1)
            newDiff = 1;
        
        if (newDiff > RETARGET_MAXDIFF)
            newDiff = RETARGET_MAXDIFF;
        
        _client->SetDifficulty(newDiff, true);
        
        return true;
    }
}
