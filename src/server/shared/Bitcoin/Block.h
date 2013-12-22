#ifndef BITCOIN_BLOCK_H_
#define BITCOIN_BLOCK_H_

#include "Common.h"
#include "Transaction.h"

#include <vector>

namespace Bitcoin
{
    class BlockHeader
    {
    public:
        uint32 version;
        std::vector<byte> prevBlockHash;
        std::vector<byte> merkleRootHash;
        uint32 time;
        uint32 bits;
        uint32 nonce;
    };
    
    class Block : public BlockHeader
    {
    public:
        std::vector<Transaction> tx;
        
        std::vector<byte> GenerateMerkle()
        {
        }
    };
}

#endif
