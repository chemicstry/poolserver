#include "ShareLimiter.h"
#include "Server.h"
#include "Client.h"
#include "Config.h"
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
        
        if (sinceLast < sConfig.Get<uint32>("RetargetInterval"))
            return true;
        
        _lastRetarget = curTime;
        
        // Check if miner is ok
        if (_totalShares > 50 && (double(_totalBadShares)/double(_totalShares)) > 0.8) {
            _client->Ban(600);
            return false;
        }
        
        while (_shares.size()) {
            if (_shares.front() > curTime - sConfig.Get<uint32>("RetargetTimeBuffer"))
                break;
            _shares.pop_front();
        }
        
        uint32 interval = std::min(curTime - _startTime, uint64(sConfig.Get<uint32>("RetargetTimeBuffer")));
        
        // Calculate shares/min
        double speed = double(_shares.size()*60) / double(interval);
        
        // Calculate difference from pool target in %
        double variance = speed / double(sConfig.Get<uint32>("RetargetSharesPerMin"));
        
        sLog.Debug(LOG_STRATUM, "Miner variance: %f speed: %f", variance, speed);
        
        // Check if we need to retarget
        if (variance*100 < sConfig.Get<uint32>("RetargetVariance"))
            return true;
        
        uint64 newDiff = double(_client->GetDifficulty()) * variance;
        
        if (newDiff < sConfig.Get<uint32>("RetargetMinDiff"))
            newDiff = sConfig.Get<uint32>("RetargetMinDiff");
        
        if (newDiff > sConfig.Get<uint32>("RetargetMaxDiff"))
            newDiff = sConfig.Get<uint32>("RetargetMaxDiff");
        
        _client->SetDifficulty(newDiff, true);
        
        return true;
    }
}
