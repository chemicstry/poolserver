#ifndef BITCOIN_BLOCK_H_
#define BITCOIN_BLOCK_H_

#include "Common.h"
#include "Transaction.h"

#include <vector>
#include <algorithm>

namespace Bitcoin
{
    class BlockHeader
    {
    public:
        uint32 version;
        BinaryData prevBlockHash;
        BinaryData merkleRootHash;
        uint32 time;
        uint32 bits;
        uint32 nonce;
        BinaryData signature;
    };
    
    class Block : public BlockHeader
    {
    public:
        // Serializable
        std::vector<Transaction> tx;
        
        // Other data
        std::vector<BinaryData> merkleTree;
        
        void BuildMerkleTree();
        // Rebuilds only left side of merkle tree
        void RebuildMerkleTree();
        BinaryData GetHash();
    };
    
    typedef boost::shared_ptr<Block> BlockPtr;
    
    // Block Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, Block& b);
    ByteBuffer& operator>>(ByteBuffer& a, Block& b);
}

#endif
