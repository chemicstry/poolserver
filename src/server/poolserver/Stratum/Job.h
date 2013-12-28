#ifndef STRATUM_JOB_H_
#define STRATUM_JOB_H_

#include "Bitcoin.h"
#include "Crypto.h"
#include "Util.h"
#include <set>

namespace Stratum
{
    class Job
    {
    public:
        Bitcoin::BlockPtr block;
        BinaryData coinbase1;
        BinaryData coinbase2;
        std::set<std::string> shares;
        
        bool SubmitShare(BinaryData share)
        {
            std::string sharestr = Util::BinToASCII(Crypto::SHA256(share));
            std::set<std::string>::iterator it = shares.find(sharestr);
            if (it == shares.end()) {
                shares.insert(sharestr);
                return true;
            } else
                return false;
        }
    };
}

#endif
