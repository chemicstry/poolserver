#include "DatabaseWorkerPool.h"

namespace MySQL
{
    bool DatabaseWorkerPool::Open(ConnectionInfo connInfo, uint8_t syncThreads, uint8_t asyncThreads)
    {
        bool res = true;
        _connectionInfo = connInfo;
        
        sLog.Info(LOG_DATABASE, "Opening MySQL Database Pool '%s'. Asynchronous threads: %u, synchronous threads: %u.", _connectionInfo.DB.c_str(), asyncThreads, syncThreads);
        
        for (uint8_t i = 0; i < syncThreads; ++i)
        {
            DatabaseConnection* conn = new DatabaseConnection(_connectionInfo);
            res &= conn->Open();
            _connections[MYSQL_CONN_SYNC].push_back(conn);
        }
        
        for (uint8_t i = 0; i < syncThreads; ++i)
        {
            DatabaseConnection* conn = new DatabaseConnection(_connectionInfo, _asyncQueue);
            res &= conn->Open();
            _connections[MYSQL_CONN_ASYNC].push_back(conn);
        }
        
        if (res)
            sLog.Info(LOG_DATABASE, "MySQL Database Pool '%s' opened successfully. %u total connections.", _connectionInfo.DB.c_str(), _connections[MYSQL_CONN_SYNC].size()+_connections[MYSQL_CONN_ASYNC].size());
        else
            sLog.Error(LOG_DATABASE, "Failed opening MySQL Database Pool to '%s'.", _connectionInfo.DB.c_str());
        
        LoadSTMT();
        
        return res;
    }
    
    void DatabaseWorkerPool::Close()
    {
        sLog.Info(LOG_DATABASE, "Closing MySQL Database Pool '%s'.", _connectionInfo.DB.c_str());
        
        // Stop worker threads
        _asyncQueue->Stop();
        
        for (uint8_t i = 0; i < _connections[MYSQL_CONN_SYNC].size(); ++i)
        {
            DatabaseConnection* conn = _connections[MYSQL_CONN_SYNC][i];
            conn->Close();
        }
        
        for (uint8_t i = 0; i < _connections[MYSQL_CONN_ASYNC].size(); ++i)
        {
            DatabaseConnection* conn = _connections[MYSQL_CONN_ASYNC][i];
            conn->Close();
        }
        
        sLog.Info(LOG_DATABASE, "Closed all connections to MySQL Database Pool '%s'.", _connectionInfo.DB.c_str());
    }
    
    bool DatabaseWorkerPool::PrepareStatement(uint32 index, const char* sql, PreparedStatementFlags flags)
    {
        if (flags & STMT_SYNC) {
            for (uint8_t i = 0; i < _connections[MYSQL_CONN_SYNC].size(); ++i) {
                DatabaseConnection* conn = _connections[MYSQL_CONN_SYNC][i];
                if (!conn->PrepareStatement(index, sql)) {
                    sLog.Error(LOG_DATABASE, "Failed to prepare statement");
                    return false;
                }
            }
        }
        
        if (flags & STMT_ASYNC) {
            for (uint8_t i = 0; i < _connections[MYSQL_CONN_ASYNC].size(); ++i) {
                DatabaseConnection* conn = _connections[MYSQL_CONN_ASYNC][i];
                if (!conn->PrepareStatement(index, sql)) {
                    sLog.Error(LOG_DATABASE, "Failed to prepare statement");
                    return false;
                }
            }
        }
        
        return true;
    }
}
