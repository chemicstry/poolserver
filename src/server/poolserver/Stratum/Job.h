#ifndef STRATUM_JOB_H_
#define STRATUM_JOB_H_

#include "Bitcoin.h"

namespace Stratum
{
    class Job
    {
    public:
        Bitcoin::BlockPtr block;
    };
}

#endif
