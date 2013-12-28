#include "Block.h"

namespace Bitcoin
{
    void Block::BuildMerkleTree()
    {
        merkleTree.clear();
        
        uint64 branches = 1;
        uint32 levels = 0;
        while (branches < tx.size()) {
            branches *= 2;
            ++levels;
        }
        
        // Used when sending merkle branches
        merkleBranches = branches;
        
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
    
    void Block::RebuildMerkleTree()
    {
        // Set coinbase tx hash
        merkleTree[0] = tx[0].GetHash();

        uint64 branches = merkleBranches;
        uint64 index = 0;
        while (branches > 1) {
            merkleTree[index+branches] = Crypto::SHA256D(Util::Join(merkleTree[index], merkleTree[index+1]));
            index += branches;
            branches /= 2;
        }
        
        // Last hash is merkle root
        merkleRootHash = merkleTree[merkleTree.size()-1];
    }
    
    BinaryData Block::GetHash()
    {
        ByteBuffer buf;
        buf << version;
        buf << prevBlockHash;
        buf << merkleRootHash;
        buf << time;
        buf << bits;
        buf << nonce;
        return Crypto::SHA256D(buf.Binary());
    }
}
