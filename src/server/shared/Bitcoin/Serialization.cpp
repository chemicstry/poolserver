#include "VarInt.h"
#include "Script.h"
#include "Transaction.h"

using namespace Bitcoin;

// VarInt
ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, VarInt& b)
{
    if (b.value < 0xfd) {
        a.Append(b.value, 1);
    } else if (b.value <= 0xffff) {
        a.Append(0xfd, 1);
        a.Append(b.value, 2);
    } else if (b.value <= 0xffffffff) {
        a.Append(0xfe, 1);
        a.Append(b.value, 4);
    } else {
        a.Append(0xff, 1);
        a.Append(b.value, 8);
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
    b.script = a.ReadBytes(size);
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
    b.hash = a.ReadBytes(32);
    b.n = a.Read<uint32>();
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
    b.n = a.Read<uint32>();
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
    b.value = a.Read<uint64>();
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
    for (uint32 i = 0; i < b.in.size(); ++i)
        a << b.in[i];
    
    // Outputs
    VarInt outsize(b.out.size());
    a << outsize;
    for (uint32 i = 0; i < b.out.size(); ++i)
        a << b.out[i];
    
    a << b.lockTime;
    return a;
}
ByteBuffer& Bitcoin::operator>>(ByteBuffer& a, Transaction& b)
{
    b.version = a.Read<uint32>();
    
    // Inputs
    VarInt insize;
    a >> insize;
    
    b.in.resize(insize);
    for (uint32 i = 0; i < insize; ++i)
        a >> b.in[i];
    
    // Outputs
    VarInt outsize;
    a >> outsize;
    
    b.out.resize(outsize);
    for (uint32 i = 0; i < outsize; ++i)
        a >> b.out[i];
    
    b.lockTime = a.Read<uint32>();
    return a;
}
