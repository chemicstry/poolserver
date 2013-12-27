#include "Server.h"
#include "Client.h"

namespace Stratum
{
    void Client::SendJob(bool clean)
    {
        if (clean)
            _jobs.clear();
        
        Job job = GetJob();
        uint64 jobid = _jobid++;
        
        _jobs[jobid] = job;
        
        // Coinbase parts
        ByteBuffer coinbasebuf;
        coinbasebuf << job.block->tx[0];
        BinaryData coinbase = coinbasebuf.Binary();
        // tx.version + tx.in[0].outpoint.hash + tx.in[0].outpoint.n + tx.script (block height)
        uint32 cbsplit = 4 + 32 + 4 + 4;
        BinaryData coinbase1 = BinaryData(coinbase.begin(), coinbase.begin() + cbsplit);
        BinaryData coinbase2 = BinaryData(coinbase.begin() + cbsplit + 8, coinbase.end()); // plus extranonce size
        
        // Build merkle branch array
        JSON merkle_branch(JSON_ARRAY);
        uint64 branches = job.block->merkleBranches;
        uint64 index = 0;
        while (branches > 1) {
            merkle_branch.Add(Util::BinToASCII(job.block->merkleTree[index+1]));
            index += branches;
            branches /= 2;
        }
        
        JSON params;
        params.Add("bf");
        params.Add(Util::BinToASCII(Util::Reverse(job.block->prevBlockHash)));
        params.Add(Util::BinToASCII(coinbase1));
        params.Add(Util::BinToASCII(coinbase2));
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
        
        SendJob(false);
    }
    
    Job Client::GetJob()
    {
        Job job;
        job.block = _server->GetWork();
        return job;
    }
}
