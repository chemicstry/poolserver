#include "Server.h"
#include "Client.h"

namespace Stratum
{
    void Client::OnMiningSubscribe(JSON msg)
    {
        _subscribed = true;
        _extranonce = _server->GetExtranonce();
        
        Job job = GetJob();
        _jobs.push_back(job);
        
        JSON response;
        response.Set("id", msg["id"].Get<uint32>());
        response.Set("error", NULL);
        
        JSON miningdiff;
        miningdiff.Add("mining.set_difficulty");
        JSON lala;
        lala.Set("", 1);
        miningdiff.Add(lala);
        
        JSON notify;
        notify.Add("mining.notify");
        notify.Add("abc");
        
        JSON something;
        something.Add(miningdiff);
        something.Add(notify);
        
        JSON result;
        result.Add(something);
        ByteBuffer noncebuf;
        noncebuf << _extranonce;
        JSON omg;
        omg.Set("", Util::BinToASCII(noncebuf.Binary()));
        result.Add(omg);
        JSON shit;
        shit.Set("", 4);
        result.Add(shit);
        
        response.Set("result", result);
        
        SendMessage(response);
    }
    
    Job Client::GetJob()
    {
        Job job;
        job.work = _server->GetWork();
        return job;
    }
}
