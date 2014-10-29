#include "ShareLimiter.h"
#include "Server.h"
#include "Client.h"
#include "Config.h"
#include "Log.h"

namespace Stratum
{
    // Returning false will stop any further share verifications (DoS prevention, etc)
    bool ShareLimiter::Submit(uint64 diff)
    {
        ++_totalShares;
        
        uint64 curTime = Util::Date();
        uint64 timeSinceRetarget = curTime - _lastRetargetTime;
        uint64 sharesSinceRetarget = _totalShares - _lastRetargetShares;
        
        _shares.push_back(ShareLimiterRecord(diff, curTime));
        
        if (timeSinceRetarget < sConfig.Get<uint32>("RetargetInterval") && sharesSinceRetarget < sConfig.Get<uint32>("RetargetSharesThreshold"))
            return true;
        
        _lastRetargetTime = curTime;
        _lastRetargetShares = _totalShares;
        
        // Check if miner is ok
        if (_totalShares > 200 && (double(_totalBadShares)/double(_totalShares)) > 0.9) {
            _client->Ban(600);
            return false;
        }
        
        // Drop old shares
        while (_shares.size()) {
            if (_shares.front().time > curTime - sConfig.Get<uint32>("RetargetTimeBuffer"))
                break;
            _shares.pop_front();
        }
        
        double totalWeighted = 0;
        for (uint32 i = 0; i < _shares.size(); ++i)
            totalWeighted += _shares[i].diff;
        
        double interval = std::min(curTime - _startTime, uint64(sConfig.Get<uint32>("RetargetTimeBuffer")));
        
        // Calculate hashrate in MH/s
        double hashrate = (MEGAHASHCONST*totalWeighted)/interval;
        
        // Calculate new diff
        uint64 newDiff = (hashrate * sConfig.Get<double>("RetargetTimePerShare")) / MEGAHASHCONST;
        
        // Check Limits
        if (newDiff < sConfig.Get<uint32>("RetargetMinDiff"))
            newDiff = sConfig.Get<uint32>("RetargetMinDiff");
        if (newDiff > sConfig.Get<uint32>("RetargetMaxDiff"))
            newDiff = sConfig.Get<uint32>("RetargetMaxDiff");
        
        // Calculate variance in %
        uint32 variance = abs(((newDiff - _client->GetDifficulty()) * 100) / _client->GetDifficulty());
        
        sLog.Debug(LOG_STRATUM, "Miner new diff: %u Variance: %u%% Hashrate: %f MH/s", newDiff, variance, hashrate);
        
        // Check if we need to retarget
        if (variance < sConfig.Get<uint32>("RetargetVariance"))
            return true;
        
        // If variance is huge, reset difficulty with job discard (DoS prevention)
        if (variance > 2000) {
            _client->SetDifficulty(newDiff, false);
            _client->SendJob(true);
            return false;
        } else
            _client->SetDifficulty(newDiff, true);
        
        return true;
    }
}
