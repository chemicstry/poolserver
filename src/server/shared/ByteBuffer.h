#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include "Common.h"
#include <vector>

class ByteBuffer
{
public:
    ByteBuffer& operator<<(ByteBuffer& b)
    {
        Append(b.vec);
        return *this;
    }
    ByteBuffer& operator<<(std::vector<byte>& b)
    {
        Append(b);
        return *this;
    }
    ByteBuffer& operator<<(uint8& b)
    {
        Append(b, 1);
        return *this;
    }
    ByteBuffer& operator<<(uint16& b)
    {
        Append(b, 2);
        return *this;
    }
    ByteBuffer& operator<<(uint32& b)
    {
        Append(b, 4);
        return *this;
    }
    ByteBuffer& operator<<(uint64& b)
    {
        Append(b, 8);
        return *this;
    }
    ByteBuffer& operator<<(int8& b)
    {
        Append(b, 1);
        return *this;
    }
    ByteBuffer& operator<<(int16& b)
    {
        Append(b, 2);
        return *this;
    }
    ByteBuffer& operator<<(int32& b)
    {
        Append(b, 4);
        return *this;
    }
    ByteBuffer& operator<<(int64& b)
    {
        Append(b, 8);
        return *this;
    }
    
    void Append(uint64 data, size_t size)
    {
        for (int i = 0; i < size; i++)
            vec.push_back(data >> (i * 8));
    }
    
    void Append(std::vector<byte> data)
    {
        vec.insert(vec.end(), data.begin(), data.end());
    }
    
    std::vector<byte> vec;
};

#endif
