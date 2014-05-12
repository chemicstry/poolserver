#include "Block.h"

namespace Bitcoin
{
    void Block::BuildMerkleTree()
    {
        merkleTree.clear();
        
        // Add transactions
        for (uint64 i = 0; i < tx.size(); ++i)
            merkleTree.push_back(tx[i].GetHash());
        
        uint32 j = 0;
        for (uint32 size = tx.size(); size > 1; size = (size+1)/2)
        {
            for (uint32 i = 0; i < size; i += 2)
            {
                uint32 i2 = std::min(i+1, size-1);
                merkleTree.push_back(Crypto::SHA256D(Util::Join(merkleTree[j+i], merkleTree[j+i2])));
            }
            
            j += size;
        }
        
        // Last hash is merkle root
        merkleRootHash = merkleTree[merkleTree.size()-1];
    }
    
    void Block::RebuildMerkleTree()
    {
        // Set coinbase tx hash
        merkleTree[0] = tx[0].GetHash();

        // Only rebuild left side of the tree
        uint32 j = 0;
        for (uint32 size = tx.size(); size > 1; size = (size+1)/2)
        {
            merkleTree[j+size] = Crypto::SHA256D(Util::Join(merkleTree[j], merkleTree[j+1]));
            j += size;
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
