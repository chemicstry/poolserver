#include "Server.h"
#include "Client.h"
#include "Config.h"
#include "BigNum.h"
#include "DataMgr.h"
#include "ShareLimiter.h"
#include "Exception.h"
#include "ServerDatabaseEnv.h"
#include <iostream>

namespace Stratum
{
    bool Client::Start()
    {
        try {
            // Get IP
            tcp::endpoint remote_ep = _socket.remote_endpoint();
            address remote_ad = remote_ep.address();
            _ip = remote_ad.to_v4().to_ulong();

            if (_server->IsBanned(_ip)) {
                sLog.Warn(LOG_STRATUM, "Blocked banned client from: %s", remote_ad.to_v4().to_string().c_str());
                Disconnect();
                return false;
            }

            // Start reading socket
            StartRead();
        } catch (std::exception& e) {
            sLog.Error(LOG_SERVER, "Exception caught while accepting client: %s", e.what());
            return false;
        }

        return true;
    }
    
    void Client::SendJob(bool clean)
    {
        if (clean)
            CleanJobs();
        
        Job* job = GetJob();
        uint32 jobid = _jobid++;
        
        _jobs[jobid] = job;
        
        std::stringstream jobss;
        jobss << std::hex << jobid;
        
        // Build merkle branch array
        JSON merkle_branch(JSON_ARRAY);
        
        uint32 j = 0;
        for (uint32 size = job->block->tx.size(); size > 1; size = (size+1)/2)
        {
            merkle_branch.Add(Util::BinToASCII(job->block->merkleTree[j+1]));
            j += size;
        }
        
        // Reverse prev block hash every 4 bytes... Makes a lot of sense...
        ByteBuffer prevhashbuf(Util::Reverse(job->block->prevBlockHash));
        std::vector<uint32> prevhash(8);
        prevhashbuf >> prevhash[7] >> prevhash[6] >> prevhash[5] >> prevhash[4] >> prevhash[3] >> prevhash[2] >> prevhash[1] >> prevhash[0];
        ByteBuffer prevhashfixed;
        prevhashfixed << prevhash[0] << prevhash[1] << prevhash[2] << prevhash[3] << prevhash[4] << prevhash[5] << prevhash[6] << prevhash[7];
        
        JSON params;
        params.Add(jobss.str());
        params.Add(Util::BinToASCII(prevhashfixed.Binary()));
        params.Add(Util::BinToASCII(job->coinbase1));
        params.Add(Util::BinToASCII(job->coinbase2));
        params.Add(merkle_branch);
        params.Add(Util::BinToASCII(Util::Reverse(ByteBuffer(job->block->version).Binary())));
        params.Add(Util::BinToASCII(Util::Reverse(ByteBuffer(job->block->bits).Binary())));
        params.Add(Util::BinToASCII(Util::Reverse(ByteBuffer(job->block->time).Binary())));
        params.Add(clean);
        
        JSON msg;
        msg["params"] = params;
        msg["id"];
        msg["method"] = "mining.notify";
        
        SendMessage(msg);
    }
    
    void Client::OnMiningSubmit(JSON msg)
    {
        JSON params = msg["params"];
        
        // check username
        std::string username = params[0].GetString();
        if (!_workers.count(username)) {
            sLog.Warn(LOG_STRATUM, "%s: Worker not authenticated", username.c_str());
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(24));
            response["error"].Add("Unauthorized worker");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        uint32 jobid;
        std::stringstream jobss;
        jobss << std::hex << params[1].GetString();
        jobss >> jobid;
        
        // Check if such job exists
        if (!_jobs.count(jobid)) {
            DataMgr::Instance()->Push(Share(_ip, username, false, "Job not found", Util::Date(), 1));
            _shareLimiter.LogBad();
            
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(21));
            response["error"].Add("Job not found");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        // Get job
        Job* job = _jobs[jobid];
        
        // Share limiter
        if (!_shareLimiter.Submit(job->diff)) {
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(20));
            response["error"].Add("Blocked by share limiter");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        BinaryData extranonce2 = Util::ASCIIToBin(params[2].GetString());
        if (extranonce2.size() != 4) {
            sLog.Warn(LOG_STRATUM, "Wrong extranonce size");
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(20));
            response["error"].Add("Wrong extranonce size");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        ByteBuffer timebuf(Util::Reverse(Util::ASCIIToBin(params[3].GetString())));
        if (timebuf.Size() != 4) {
            sLog.Warn(LOG_STRATUM, "Wrong ntime size");
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(20));
            response["error"].Add("Wrong ntime size");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        ByteBuffer noncebuf(Util::Reverse(Util::ASCIIToBin(params[4].GetString())));
        if (noncebuf.Size() != 4) {
            sLog.Warn(LOG_STRATUM, "Wrong nonce size");
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(20));
            response["error"].Add("Wrong nonce size");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        // Pack two 32bit ints into 64bit
        ByteBuffer sharebuf;
        sharebuf << noncebuf << extranonce2;
        uint64 share;
        sharebuf >> share;
        
        sLog.Debug(LOG_STRATUM, "Job::SubmitShare: Nonce: %s, Extranonce: %s, Share: %u", Util::BinToASCII(noncebuf.Binary()).c_str(), Util::BinToASCII(extranonce2).c_str(), share);
        
        if (!job->SubmitShare(share)) {
            sLog.Warn(LOG_STRATUM, "%s: Duplicate share", username.c_str());
            DataMgr::Instance()->Push(Share(_ip, username, false, "Duplicate share", Util::Date(), job->diff));
            _shareLimiter.LogBad();
            
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(22));
            response["error"].Add("Duplicate share");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        // Copy block we are working on
        Bitcoin::Block block = *job->block;
        
        // Start assembling the block
        timebuf >> block.time;
        noncebuf >> block.nonce;
        
        // Assemble coinbase
        Bitcoin::Transaction coinbasetx;
        ByteBuffer coinbasebuf;
        coinbasebuf << job->coinbase1 << _extranonce << extranonce2 << job->coinbase2;
        coinbasebuf >> coinbasetx;
        
        // Set coinbase tx
        block.tx[0] = coinbasetx;
        
        // Rebuilds only left side of merkle tree
        block.RebuildMerkleTree();
        
        // Get block hash
        BinaryData hash = block.GetHash();
        
        // Get block target
        BigInt target(Util::BinToASCII(Util::Reverse(hash)), 16);
        
        // Check if difficulty meets job diff
        if (target > job->jobTarget) {
            sLog.Warn(LOG_STRATUM, "%s: Share above target", username.c_str());
            DataMgr::Instance()->Push(Share(_ip, username, false, "Share above target", Util::Date(), job->diff));
            _shareLimiter.LogBad();
            
            JSON response;
            response["id"] = msg["id"];
            response["result"];
            response["error"].Add(int64(23));
            response["error"].Add("Share above target");
            response["error"].Add(JSON());
            SendMessage(response);
            return;
        }
        
        // Check if block meets criteria
        if (target <= job->blockTarget) {
            sLog.Info(LOG_STRATUM, "We have found a block candidate!");
            
            // copy job diff because job will be deleted after submiting share by block template update
            uint64 jobDiff = job->diff;
            
            if (_server->SubmitBlock(block)) {
                std::string query("INSERT INTO `shares` (`rem_host`, `username`, `our_result`, `upstream_result`, `reason`, `solution`, `time`, `difficulty`) VALUES ");
                query += Util::FS("(INET_NTOA(%u), '%s', 1, 1, '', '%s', FROM_UNIXTIME(%u), %u)", _ip, username.c_str(), Util::BinToASCII(Util::Reverse(hash)).c_str(), Util::Date(), jobDiff);
                sDatabase.ExecuteAsync(query.c_str());
                
                JSON response;
                response["id"] = msg["id"];
                response["result"] = true;
                response["error"];
                SendMessage(response);
                return;
            }
        } else {
            DataMgr::Instance()->Push(Share(_ip, username, true, "", Util::Date(), job->diff));
            
            JSON response;
            response["id"] = msg["id"];
            response["result"] = true;
            response["error"];
            SendMessage(response);
            return;
        }
    }
    
    void Client::OnMiningSubscribe(JSON msg)
    {
        _subscribed = true;
        
        // Get extranonce from server
        _extranonce = _server->GetExtranonce();
        ByteBuffer noncebuf;
        noncebuf << _extranonce;
        
        JSON notify;
        notify.Add("mining.notify");
        notify.Add("ae6812eb4cd7735a302a8a9dd95cf71f");
        
        JSON result;
        result.Add(notify);
        result.Add(Util::BinToASCII(noncebuf.Binary()));
        result.Add(int64(4));
        
        JSON response;
        response["id"] = msg["id"];
        response["result"] = result;
        response["error"];
        
        SendMessage(response);
        
        SendJob(false);
    }
    
    void Client::OnMiningAuthorize(JSON msg)
    {
        std::string username = msg["params"][0].GetString();
        std::string password = msg["params"][1].GetString();
        
        MySQL::QueryResult result = sDatabase.Query(Util::FS("SELECT `id`, `mindiff` FROM `pool_worker` WHERE `username` = '%s' and `password` = '%s'", sDatabase.Escape(username).c_str(), sDatabase.Escape(password).c_str()).c_str());
        
        if (result) {
            _workers.insert(username);
            
            MySQL::Field* fields = result->FetchRow();
            _minDiff = fields[1].Get<uint32>();
            
            if (_diff < _minDiff)
                SetDifficulty(_minDiff);
            
            JSON response;
            response["id"] = msg["id"];
            response["error"];
            response["result"] = true;
            SendMessage(response);
        } else {
            JSON response;
            response["id"] = msg["id"];
            response["error"].Add(int64(20));
            response["error"].Add("Authentication failed");
            response["error"].Add(JSON());
            response["result"] = true;
            SendMessage(response);
        }
    }
    
    Job* Client::GetJob()
    {
        Job* job = new Job();
        job->block = _server->GetWork();
        job->diff = _diff;
        job->jobTarget = Bitcoin::DiffToTarget(job->diff);
        job->blockTarget = Bitcoin::TargetFromBits(job->block->bits);
        
        // Serialize transaction
        ByteBuffer coinbasebuf;
        coinbasebuf << job->block->tx[0];
        BinaryData coinbase = coinbasebuf.Binary();
        
        // Split coinbase
        // tx.version                   - 4 bytes
        // tx.in[0].outpoint.hash       - 32 bytes
        // tx.in[0].outpoint.n          - 4 bytes
        // tx.script (varint)           - 2 bytes
        // tx.script (block height)     - 4 bytes
        uint32 cbsplit = 4 + 32 + 4 + 2 + 4;
        job->coinbase1 = BinaryData(coinbase.begin(), coinbase.begin() + cbsplit);
        job->coinbase2 = BinaryData(coinbase.begin() + cbsplit + 8, coinbase.end()); // plus extranonce size
        
        return job;
    }
    
    void Client::Ban(uint32 time)
    {
        _server->Ban(_ip, time);
        Disconnect();
    }
    
    void Client::Disconnect()
    {
        _server->Disconnect(shared_from_this());
    }
    
    void Client::_OnReceive(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (!error) {
            std::istream is(&_recvBuffer);
            char c;
            while (is.get(c)) {
                if (c == '\n') {
                    sLog.Debug(LOG_STRATUM, "Received message: %s", _recvMessage.c_str());
                    
                    // Redirect getwork
                    if (!_subscribed) {
                        if (_recvMessage.compare("POST / HTTP/1.1\r") == 0) {
                            RedirectGetwork();
                        }
                    }
                    
                    try {
                        OnMessage(JSON::FromString(_recvMessage));
                    } catch (std::exception& e) {
                        sLog.Debug(LOG_STRATUM, "Exception caught while parsing json: %s", e.what());
                    }
                    
                    _recvMessage.clear();
                } else
                    _recvMessage += c;
            }
            
            // Check if message doesn't exceed maximum packet size
            if (_recvMessage.size() > MAX_PACKET)
                Disconnect();
            
            StartRead();
        } else {
            // Client disconnected
            if ((error == asio::error::eof) || (error == asio::error::connection_reset)) {
                Disconnect();
            }
        }
    }
    
    void Client::_OnSend(const boost::system::error_code& error)
    {
        if (error) {
            // Client disconnected
            if ((error == asio::error::eof) || (error == asio::error::connection_reset)) {
                Disconnect();
            }
        }
    }
}
