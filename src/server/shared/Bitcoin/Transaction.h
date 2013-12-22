#ifndef BITCOIN_TRANSACTION_H_
#define BITCOIN_TRANSACTION_H_

#include "Common.h"
#include "ByteBuffer.h"
#include "VarInt.h"
#include "Script.h"

namespace Bitcoin
{
    class OutPoint
    {
    public:
        std::vector<byte> hash;
        uint32 n;
    };
    
    ByteBuffer& operator<<(ByteBuffer& a, OutPoint& b);
    
    class TxIn
    {
    public:
        OutPoint prevout;
        Script script;
        uint32 n;
    };
    
    ByteBuffer& operator<<(ByteBuffer& a, TxIn& b);
    
    class TxOut
    {
    public:
        int64 value;
        Script scriptPubKey;
    };
    
    ByteBuffer& operator<<(ByteBuffer& a, TxOut& b);
    
    class Transaction
    {
    public:
        uint32 version;
        std::vector<TxIn> in;
        std::vector<TxOut> out;
        uint32 lockTime;
    };
    
    ByteBuffer& operator<<(ByteBuffer& a, Transaction& b);
}

#endif
