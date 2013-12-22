#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include "Common.h"
#include <vector>

class ByteBuffer
{
public:
    ByteBuffer(): pointer(0) {}
    ByteBuffer(std::vector<byte> data): pointer(0), vec(data) {}
    
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
    
    std::vector<byte> ReadBytes(size_t size)
    {
        if (vec.size() < pointer+size)
            return std::vector<byte>();
        
        pointer += size;
        return std::vector<byte>(vec.begin()+pointer-size, vec.begin()+pointer);
    }
    
    template<typename T>
    T Read()
    {
        size_t size = sizeof(T);
        
        if (vec.size() < pointer+size)
            return NULL;
        
        T data = 0;
        for (uint32 i = 0; i < size; ++i)
            data += vec[pointer+i]<<(i*8);
        
        pointer += size;
        
        return data;
    }
    
    uint64 pointer;
    std::vector<byte> vec;
};

#endif
