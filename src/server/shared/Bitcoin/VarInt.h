#ifndef BITCOIN_VARINT_H_
#define BITCOIN_VARINT_H_

#include "Common.h"
#include "ByteBuffer.h"

namespace Bitcoin
{
    class VarInt
    {
    public:
        VarInt(uint64 data) : value(data) {}
        uint64 value;
        
        friend ByteBuffer& operator<<(ByteBuffer& a, VarInt& b);
    };
    
    ByteBuffer& operator<<(ByteBuffer& a, VarInt& b);
}

#endif
