#ifndef BITCOIN_TRANSACTION_H_
#define BITCOIN_TRANSACTION_H_

#include "Common.h"
#include "BigNum.h"
#include "Script.h"

namespace Bitcoin
{
    class OutPoint
    {
        std::vector<byte> hash;
        uint32 n;
    };
    
    class InPoint
    {
    };
    
    class TxIn
    {
        OutPoint prevout;
        Script script;
        uint32 n;
    };
    
    class TxOut
    {
        int64 value;
        Script scriptPubKey;
    };
    
    class Transaction
    {
        uint32 version;
        std::vector<TxIn> in;
        std::vector<TxOut> out;
        uint32 lockTime;
    };
}

#endif
