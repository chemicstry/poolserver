#include "Server.h"
#include "Client.h"
#include "BigNum.h"
#include <iostream>

namespace Stratum
{
    void Client::SendJob(bool clean)
    {
        if (clean)
            _jobs.clear();
        
        Job job = GetJob();
        uint32 jobid = _jobid++;
        
        _jobs[jobid] = job;
        
        // Build merkle branch array
        JSON merkle_branch(JSON_ARRAY);
        uint64 branches = job.block->merkleBranches;
        uint64 index = 0;
        while (branches > 1) {
            merkle_branch.Add(Util::BinToASCII(job.block->merkleTree[index+1]));
            index += branches;
            branches /= 2;
        }
        
        // Reverse prev block hash every 4 bytes... Makes a lot of sense...
        ByteBuffer prevhashbuf(Util::Reverse(job.block->prevBlockHash));
        std::vector<uint32> prevhash(8);
        prevhashbuf >> prevhash[7] >> prevhash[6] >> prevhash[5] >> prevhash[4] >> prevhash[3] >> prevhash[2] >> prevhash[1] >> prevhash[0];
        ByteBuffer prevhashfixed;
        prevhashfixed << prevhash[0] << prevhash[1] << prevhash[2] << prevhash[3] << prevhash[4] << prevhash[5] << prevhash[6] << prevhash[7];
        
        JSON params;
        params.Add(Util::BinToASCII(ByteBuffer(jobid).Binary()));
        params.Add(Util::BinToASCII(prevhashfixed.Binary()));
        params.Add(Util::BinToASCII(job.coinbase1));
        params.Add(Util::BinToASCII(job.coinbase2));
        params.Add(merkle_branch);
        params.Add(Util::BinToASCII(Util::Reverse(ByteBuffer(job.block->version).Binary())));
        params.Add(Util::BinToASCII(Util::Reverse(ByteBuffer(job.block->bits).Binary())));
        params.Add(Util::BinToASCII(Util::Reverse(ByteBuffer(job.block->time).Binary())));
        params.Add(clean);
        
        JSON msg;
        msg["params"] = params;
        msg["id"];
        msg["method"] = "mining.notify";
        
        SendMessage(msg);
    }
    
    void Client::OnMiningSubmit(JSON msg)
    {
        //msg = JSON::FromString("{\"params\": [\"slush.miner1\", \"00000000\", \"00000001\", \"504e86ed\", \"b2957c02\"], \"id\": 4, \"method\": \"mining.submit\"}");
        JSON params = msg["params"];
        
        std::string username = params[0].GetString();
        uint32 jobid;
        ByteBuffer jobbuf(Util::ASCIIToBin(params[1].GetString()));
        jobbuf >> jobid;
        
        if (!_jobs.count(jobid)) {
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(21));
            response["error"].Add("Job not found");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        // check username
        
        BinaryData extranonce2 = Util::ASCIIToBin(params[2].GetString());
        if (extranonce2.size() != 4) {
            sLog.Error(LOG_SERVER, "Wrong extranonce size");
            return;
        }
        
        ByteBuffer timebuf(Util::Reverse(Util::ASCIIToBin(params[3].GetString())));
        if (timebuf.Size() != 4) {
            sLog.Error(LOG_SERVER, "Wrong ntime size");
            return;
        }
        
        ByteBuffer noncebuf(Util::Reverse(Util::ASCIIToBin(params[4].GetString())));
        if (noncebuf.Size() != 4) {
            sLog.Error(LOG_SERVER, "Wrong nonce size");
            return;
        }
        
        // Get job
        Job& job = _jobs[jobid];
        
        ByteBuffer share;
        share << extranonce2 << timebuf << noncebuf;
        if (!job.SubmitShare(share.Binary())) {
            sLog.Error(LOG_SERVER, "Duplicate share");
            return;
        }
        
        // Get block we are working on
        Bitcoin::Block block = *job.block;
        
        timebuf >> block.time;
        noncebuf >> block.nonce;
        
        Bitcoin::Transaction coinbasetx;
        ByteBuffer coinbasebuf;
        coinbasebuf << job.coinbase1 << _extranonce << extranonce2 << job.coinbase2;
        coinbasebuf >> coinbasetx;
        
        block.tx[0] = coinbasetx;
        
        ByteBuffer test;
        test << coinbasetx;
        sLog.Info(LOG_SERVER, "Coinbase: %s", Util::BinToASCII(test.Binary()).c_str());
        sLog.Info(LOG_SERVER, "Coinbase hash1: %s", Util::BinToASCII(Crypto::SHA256D(coinbasebuf.Binary())).c_str());
        sLog.Info(LOG_SERVER, "Coinbase hash2: %s", Util::BinToASCII(coinbasetx.GetHash()).c_str());
        
        block.RebuildMerkleTree();
        sLog.Info(LOG_SERVER, "Merklehash: %s", Util::BinToASCII(block.merkleRootHash).c_str());
        BinaryData hash = block.GetHash();
        
        sLog.Info(LOG_SERVER, "Block hash: %s", Util::BinToASCII(hash).c_str());
        
        BigInt target(Util::BinToASCII(Util::Reverse(hash)), 16);
        BigInt criteria(Bitcoin::TargetFromBits(block.bits));
        BigInt diff = Bitcoin::TargetConvert(criteria);
        std::cout << "Target: " << target << std::endl;
        std::cout << "Criteria: " << criteria << std::endl;
        std::cout << "Diff: " << diff << std::endl;
        
        if (target <= criteria) {
            sLog.Info(LOG_SERVER, "We have found a block candidate!");
            
            // Serialize block
            ByteBuffer blockbuf;
            blockbuf << block;
            
            JSON params;
            params.Add(Util::BinToASCII(blockbuf.Binary()));
            JSON response = _bitcoinrpc->Query("submitblock", params);
        } 
    }
    
    void Client::OnMiningSubscribe(JSON msg)
    {
        _subscribed = true;
        _extranonce = _server->GetExtranonce();
        
        JSON notify;
        notify.Add("mining.notify");
        notify.Add("ae6812eb4cd7735a302a8a9dd95cf71f");
        
        JSON result;
        result.Add(notify);
        ByteBuffer noncebuf;
        noncebuf << _extranonce;
        result.Add(Util::BinToASCII(noncebuf.Binary()));
        result.Add(int64(4));
        
        JSON response;
        response["id"] = msg["id"].GetInt();
        response["result"] = result;
        response["error"];
        
        SendMessage(response);
        
        SendJob(true);
    }
    
    void Client::OnMiningAuthorize(JSON msg)
    {
        sLog.Info(LOG_SERVER, "Test: %s", msg["params"].ToString().c_str());
        std::string username = msg["params"][0].GetString();
        std::string password = msg["params"][1].GetString();
        JSON response;
        response["id"] = msg["id"].GetInt();
        response["error"];
        response["result"] = true;
        SendMessage(response);
    }
    
    Job Client::GetJob()
    {
        Job job;
        job.block = _server->GetWork();
        
        // Coinbase parts
        ByteBuffer coinbasebuf;
        coinbasebuf << job.block->tx[0];
        BinaryData coinbase = coinbasebuf.Binary();
        // tx.version + tx.in[0].outpoint.hash + tx.in[0].outpoint.n + tx.script (size) + tx.script (block height)
        uint32 cbsplit = 4 + 32 + 4 + 2 + 4;
        job.coinbase1 = BinaryData(coinbase.begin(), coinbase.begin() + cbsplit);
        job.coinbase2 = BinaryData(coinbase.begin() + cbsplit + 8, coinbase.end()); // plus extranonce size
        
        return job;
    }
}
