#include "DataMgr.h"
#include "ServerDatabaseEnv.h"
#include "Util.h"
#include "Log.h"

DataMgr* DataMgr::singleton = 0;

void DataMgr::Upload()
{
    sLog.Info(LOG_SERVER, "Uploading %u shares", Size());
    
    uint32 bulkCount = sConfig.Get<uint32>("ShareUploadBulkCount");
    
    while (Size() > sConfig.Get<uint32>("ShareUploadMinCount"))
    {
        std::string query("INSERT INTO `shares` (`rem_host`, `username`, `our_result`, `upstream_result`, `reason`, `time`, `difficulty`) VALUES ");
        for (int i = 0; i < bulkCount; ++i)
        {
            Share share = Pop();
            
            query += Util::FS("(INET_NTOA(%u), '%s', %u, 0, '%s', FROM_UNIXTIME(%u), %u)", share.host, share.username.c_str(), share.result, share.reason.c_str(), share.time, share.diff);
            
            if (!Size())
                break;
            
            if (i != bulkCount-1)
                query += ',';
        }
        
        sLog.Debug(LOG_SERVER, "Query: %s", query.c_str());
        
        sDatabase.ExecuteAsync(query.c_str());
    }
}
