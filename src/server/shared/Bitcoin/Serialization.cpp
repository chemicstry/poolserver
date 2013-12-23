#include "VarInt.h"
#include "Script.h"
#include "Transaction.h"
#include "Block.h"

using namespace Bitcoin;

// VarInt
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, VarInt& b)
{
    if (b.value < 0xfd) {
        a.Append<uint8>(b.value);
    } else if (b.value <= 0xffff) {
        a.Append<uint8>(0xfd);
        a.Append<uint16>(b.value);
    } else if (b.value <= 0xffffffff) {
        a.Append<uint8>(0xfe);
        a.Append<uint32>(b.value);
    } else {
        a.Append<uint8>(0xff);
        a.Append<uint64>(b.value);
    }
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, VarInt& b)
{
    uint8 size = a.Read<uint8>();
    
    if (size < 0xfd)
        b.value = size;
    else if (size == 0xfd)
        b.value = a.Read<uint16>();
    else if (size == 0xfe)
        b.value = a.Read<uint32>();
    else
        b.value = a.Read<uint64>();
}

// Script
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, Script& b)
{
    VarInt size(b.script.size());
    a << size;
    a << b.script;
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, Script& b)
{
    VarInt size;
    a >> size;
    b.script = a.ReadBinary(size);
    return a;
}

// OutPoint
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, OutPoint& b)
{
    a << b.hash;
    a << b.n;
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, OutPoint& b)
{
    b.hash = a.ReadBinary(32);
    a >> b.n;
    return a;
}

// TxIn
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, TxIn& b)
{
    a << b.prevout;
    a << b.script;
    a << b.n;
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, TxIn& b)
{
    a >> b.prevout;
    a >> b.script;
    a >> b.n;
    return a;
}

// TxOut
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, TxOut& b)
{
    a << b.value;
    a << b.scriptPubKey;
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, TxOut& b)
{
    a >> b.value;
    a >> b.scriptPubKey;
    return a;
}

// Transaction
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, Transaction& b)
{
    a << b.version;
    
    // Inputs
    VarInt insize(b.in.size());
    a << insize;
    for (uint64 i = 0; i < insize; ++i)
        a << b.in[i];
    
    // Outputs
    VarInt outsize(b.out.size());
    a << outsize;
    for (uint64 i = 0; i < outsize; ++i)
        a << b.out[i];
    
    a << b.lockTime;
    
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, Transaction& b)
{
    a >> b.version;
    
    // Inputs
    VarInt insize;
    a >> insize;
    
    b.in.resize(insize);
    for (uint64 i = 0; i < insize; ++i)
        a >> b.in[i];
    
    // Outputs
    VarInt outsize;
    a >> outsize;
    
    b.out.resize(outsize);
    for (uint64 i = 0; i < outsize; ++i)
        a >> b.out[i];
    
    a >> b.lockTime;
    
    return a;
}

// Block
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, Block& b)
{
    a << b.version;
    a << b.prevBlockHash;
    a << b.merkleRootHash;
    a << b.time;
    a << b.bits;
    a << b.nonce;
    
    // Serialize transactions
    VarInt txcount(b.tx.size());
    a << txcount;
    for (uint64 i = 0; i < txcount; ++i)
        a << b.tx[i];
    
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, Block& b)
{
    a >> b.version;
    b.prevBlockHash = a.ReadBinary(32);
    b.merkleRootHash = a.ReadBinary(32);
    a >> b.time;
    a >> b.bits;
    a >> b.nonce;
    
    VarInt txcount;
    a >> txcount;
    
    b.tx.resize(txcount);
    for (uint64 i = 0; i < txcount; ++i)
        a >> b.tx[i];
    
    return a;
}
