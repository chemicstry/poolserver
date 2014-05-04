#ifndef SHARE_H_
#define SHARE_H_

#include <string>

#include "Common.h"

class Share
{
public:
    Share(uint32 ahost, std::string ausername, bool aresult, std::string areason, uint64 atime, uint64 adiff):
    host(ahost), username(ausername), result(aresult), reason(areason), time(atime), diff(adiff) {}
    
    uint64 host;
    std::string username;
    bool result;
    std::string reason;
    uint64 time;
    uint64 diff;
};

#endif
