#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include "Common.h"
#include <vector>

class ByteBuffer
{
public:
    ByteBuffer(): pointer(0) {}
    
    template<typename T>
    ByteBuffer(T data): pointer(0)
    {
        Append<T>(data);
    }
    
    ByteBuffer(BinaryData data): pointer(0), vec(data) {}
    
    ByteBuffer& operator<<(ByteBuffer& b)
    {
        Append(b.vec);
        return *this;
    }
    ByteBuffer& operator<<(BinaryData& b)
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
    
    void Append(BinaryData data)
    {
        vec.insert(vec.end(), data.begin(), data.end());
    }
    
    BinaryData ReadBinary(size_t size)
    {
        if (vec.size() < pointer+size)
            return BinaryData();
        
        pointer += size;
        return BinaryData(vec.begin()+pointer-size, vec.begin()+pointer);
    }
    
    template<typename T>
    T Read()
    {
        size_t size = sizeof(T);
        
        if (vec.size() < pointer+size)
            return 0;
        
        T data = 0;
        for (uint64 i = 0; i < size; ++i)
            data += (T)vec[pointer+i]<<(i*8);
        
        pointer += size;
        
        return data;
    }
    
    BinaryData Binary()
    {
        return vec;
    }
    
    uint64 Size()
    {
        return vec.size();
    }
    
    uint64 pointer;
    BinaryData vec;
};

#endif
