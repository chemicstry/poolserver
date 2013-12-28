#ifndef BITCOIN_H_
#define BITCOIN_H_

#include "Block.h"
#include "Transaction.h"
#include "VarInt.h"
#include "Script.h"
#include "BigNum.h"
#include <gmpxx.h>

namespace Bitcoin
{
    inline BigInt TargetToDiff(BigInt val)
    {
        static BigInt c("0x00000000ffff0000000000000000000000000000000000000000000000000000");
        return (c / val);
    }
    
    inline BigInt DiffToTarget(BigInt val)
    {
        // Algebra says that it is the same!
        return TargetToDiff(val);
    }
    
    inline BigInt TargetFromBits(uint32 bits)
    {
        uint8 nbytes = (bits >> 24) & 0xFF;
        mpz_t power;
        mpz_init(power);
        mpz_ui_pow_ui(power, 2, 8 * (nbytes - 3));
        return BigInt(bits & 0xFFFFFF) * BigInt(power);
    }
}

#endif
