#ifndef BITCOIN_H_
#define BITCOIN_H_

namespace Bitcoin
{
    class TxIn
    {
    };
    
    class TxOut
    {
    };
    
    class Transaction
    {
        uint32 Version;
        std::vector<TxIn> in;
        std::vector<TxOut> out;
        uint32 LockTime;
    };
    
    class BlockHeader
    {
        uint32 Version;
        std::array<char, 32> PrevBlockHash;
        std::array<char, 32> MerkleRootHash;
        uint32 Time;
        uint32 Bits;
        uint32 Nonce;
    };
    
    class Block : public BlockHeader
    {
        std::vector<Transaction> tx;
    };
    
    class BlockTemplate
}

#endif
