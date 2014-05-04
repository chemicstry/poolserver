#include "ShareLimiter.h"
#include "Server.h"
#include "Client.h"

namespace Stratum
{
    // Returning false will stop any further share verifications (DoS prevention, etc)
    bool ShareLimiter::Submit()
    {
        uint64 curTime = Util::Date();
        uint64 sinceLast = curTime - _lastRetarget;
        
        _shares.push_back(curTime);
        
        if (sinceLast < RETARGET_INTERVAL)
            return true;
        
        while (_shares.size() && (_shares.front() < curTime - RETARGET_TIME_BUFFER))
            _shares.pop_front();
        
        uint32 interval = std::min(curTime - _startTime, uint64(RETARGET_TIME_BUFFER));
        
        // Calculate shares/min
        double speed = (_shares.size()*60) / interval;
        
        // Calculate difference from pool target in %
        double variance = (speed - RETARGET_SHARES_PER_MIN) / RETARGET_SHARES_PER_MIN;
        
        // Check if we need to retarget
        if (std::abs(variance)*100 < RETARGET_VARIANCE)
            return true;
        
        uint64 newDiff = double(_client->GetDifficulty()) * variance;
        
        _client->SetDifficulty(newDiff, true);
        
        return true;
    }
}
