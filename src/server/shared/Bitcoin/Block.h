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
    };
    
    class Block : public BlockHeader
    {
    public:
        // Serializable
        std::vector<Transaction> tx;
        
        // Other data
        std::vector<BinaryData> merkleTree;
        
        void BuildMerkleTree()
        {
            merkleTree.clear();
            
            uint64 branches = 1;
            uint32 levels = 0;
            while (branches < tx.size()) {
                branches *= 2;
                ++levels;
            }
            
            // Add transactions
            for (uint64 i = 0; i < branches; ++i)
                merkleTree.push_back(tx[std::min(i, tx.size()-1)].GetHash());
            
            uint32 merkleIndex = 0;
            for (uint32 level = levels; level > 0; --level)
            {
                // Decrease before calculating because bottom level is transactions
                branches /= 2;
                
                for (uint32 branch = 0; branch < branches; ++branch)
                    merkleTree.push_back(Crypto::SHA256D(Util::Join(merkleTree[merkleIndex++], merkleTree[merkleIndex++])));
            }
            
            // Last hash is merkle root
            merkleRootHash = merkleTree[merkleTree.size()-1];
        }
    };
    
    typedef boost::shared_ptr<Block> BlockPtr;
    
    // Block Serialization (Implementation in Serialization.cpp)
    ByteBuffer& operator<<(ByteBuffer& a, Block& b);
    ByteBuffer& operator>>(ByteBuffer& a, Block& b);
}

#endif
