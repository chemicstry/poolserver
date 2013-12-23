#ifndef BITCOIN_SCRIPT_H_
#define BITCOIN_SCRIPT_H_

#include "Common.h"
#include "ByteBuffer.h"
#include "VarInt.h"
#include <boost/range/join.hpp>
#include <vector>

namespace Bitcoin
{
    enum ScriptOPCode
    {
        OP_0                = 0x00,
        OP_FALSE            = OP_0,
        OP_PUSHDATA1        = 0x4C,
        OP_PUSHDATA2        = 0x4D,
        OP_PUSHDATA4        = 0x4E,
        OP_CHECKSIG         = 0xAC,
    };
    
    class Script
    {
    public:
        Script() {}
        Script(BinaryData data) : script(data) {}
        
        BinaryData script;
        
        const Script operator+(const Script& other)
        {
            BinaryData tmp = script;
            tmp.insert(tmp.end(), other.script.begin(), other.script.end());
            return Script(tmp);
        }
        
        const Script operator+(const BinaryData data)
        {
            Script tmp(script);
            
            size_t size = data.size();
            
            if (size >= 1 && size <= 75) {
                // We know size!
                tmp.script.resize(size+1);
                
                // Push data size
                tmp.script.push_back((byte)size);
                
                // We start from 1 because index 0 is size opcode
                for (uint16 i = 1; i <= size; ++i)
                    tmp.script[i] = (byte)data[i];
            }
            
            return tmp;
        }
        
        const Script operator+(const ScriptOPCode op)
        {
            Script tmp(script);
            tmp.script.push_back((byte)op);
            return tmp;
        }
    };
    
    // Script Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, Script& b);
    ByteBuffer& operator>>(ByteBuffer& a, Script& b);
}

#endif
