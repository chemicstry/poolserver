#ifndef STRATUM_JOB_H_
#define STRATUM_JOB_H_

#include "Bitcoin.h"
#include "Crypto.h"
#include "Util.h"
#include "BigNum.h"
#include <set>

namespace Stratum
{
    class Job
    {
    public:
        uint64 diff;
        Bitcoin::BlockPtr block;
        BinaryData coinbase1;
        BinaryData coinbase2;
        std::set<uint64> shares;
        BigInt blockCriteria;
        BigInt target;
        
        // Submits share to a job
        // Returns false if the same share already exists
        bool SubmitShare(uint64 share)
        {
            std::set<uint64>::iterator it = shares.find(share);
            if (it == shares.end()) {
                shares.insert(share);
                return true;
            } else
                return false;
        }
    };
}

#endif
