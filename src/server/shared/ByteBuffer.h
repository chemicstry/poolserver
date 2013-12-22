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
    
    template<typename T>
    ByteBuffer& operator<<(T& b)
    {
        Append<T>(b);
        return *this;
    }
    
    template<typename T>
    ByteBuffer& operator>>(T& b)
    {
        b = Read<T>();
        return *this;
    }
    
    template<typename T>
    void Append(T data)
    {
        for (uint8 i = 0; i < sizeof(T); ++i)
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
        for (uint64 i = 0; i < size; ++i)
            data += (T)vec[pointer+i]<<(i*8);
        
        pointer += size;
        
        return data;
    }
    
    std::vector<byte> Bytes()
    {
        return vec;
    }
    
    uint64 pointer;
    std::vector<byte> vec;
};

#endif
