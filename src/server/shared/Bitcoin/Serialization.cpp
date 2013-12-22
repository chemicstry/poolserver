#include "VarInt.h"
#include "Script.h"
#include "Transaction.h"

using namespace Bitcoin;

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

ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, Script& b)
{
    VarInt size(b.script.size());
    a << size;
    a << b.script;
    return a;
}

ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, OutPoint& b)
{
    a << b.hash;
    a << b.n;
    return a;
}

ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, TxIn& b)
{
    a << b.prevout;
    a << b.script;
    a << b.n;
    return a;
}

ByteBuffer& Bitcoin::operator<<(ByteBuffer& a, TxOut& b)
{
    a << b.value;
    a << b.scriptPubKey;
    return a;
}

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
