#ifndef BITCOIN_H_
#define BITCOIN_H_

#include "Block.h"
#include "Transaction.h"
#include "VarInt.h"
#include "Script.h"
#include "BigNum.h"
#include "Util.h"
#include <gmpxx.h>

namespace Bitcoin
{
    inline BigInt TargetToDiff(BigInt val)
    {
        static BigInt c("0x00000000ffff0000000000000000000000000000000000000000000000000000");
        return (c / val);
    }
    
    inline BigInt DiffToTarget(BigInt val)
    {
        // Algebra says that it is the same!
        return TargetToDiff(val);
    }
    
    inline BigInt TargetFromBits(uint32 bits)
    {
        uint8 nbytes = (bits >> 24) & 0xFF;
        mpz_t power;
        mpz_init(power);
        mpz_ui_pow_ui(power, 2, 8 * (nbytes - 3));
        return BigInt(bits & 0xFFFFFF) * BigInt(power);
    }
    
    inline Transaction CreateCoinbaseTX(uint32 blockHeight, BinaryData pubkey, int64 value, BinaryData extras)
    {
        // Extranonce placeholder
        BinaryData extranonce_ph(8, 0);
        ByteBuffer scriptsig;
        scriptsig << blockHeight << extranonce_ph << extras;
        
        Bitcoin::OutPoint outpoint;
        outpoint.hash.resize(32, 0);
        outpoint.n = 0xFFFFFFFF;
        
        TxIn txin;
        txin.prevout = outpoint;
        txin.script = scriptsig.Binary();
        txin.n = 0;
        
        ByteBuffer temp;
        temp.Append<uint8>(pubkey.size());
        temp << pubkey;
        temp.Append<uint8>(Bitcoin::OP_CHECKSIG);
        
        TxOut txout;
        txout.value = value;
        txout.scriptPubKey = Bitcoin::Script(temp.Binary());
        
        Transaction tx;
        tx.version = 1;
        tx.time = Util::Date();
        tx.in.push_back(txin);
        tx.out.push_back(txout);
        tx.lockTime = 0;
        
        return tx;
    }
}

#endif
