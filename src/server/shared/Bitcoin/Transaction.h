#ifndef BITCOIN_TRANSACTION_H_
#define BITCOIN_TRANSACTION_H_

#include "Common.h"
#include "ByteBuffer.h"
#include "VarInt.h"
#include "Script.h"
#include "Crypto.h"

namespace Bitcoin
{
    class OutPoint
    {
    public:
        BinaryData hash;
        uint32 n;
    };
    
    // OutPoint Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, OutPoint& b);
    ByteBuffer& operator>>(ByteBuffer& a, OutPoint& b);
    
    class TxIn
    {
    public:
        OutPoint prevout;
        Script script;
        uint32 n;
    };
    
    // TxIn Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, TxIn& b);
    ByteBuffer& operator>>(ByteBuffer& a, TxIn& b);
    
    class TxOut
    {
    public:
        int64 value;
        Script scriptPubKey;
    };
    
    // TxOut Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, TxOut& b);
    ByteBuffer& operator>>(ByteBuffer& a, TxOut& b);
    
    class Transaction
    {
    public:
        uint32 version;
        std::vector<TxIn> in;
        std::vector<TxOut> out;
        uint32 lockTime;
        
        BinaryData GetHash();
    };
    
    // Transaction Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, Transaction& b);
    ByteBuffer& operator>>(ByteBuffer& a, Transaction& b);
}

#endif
