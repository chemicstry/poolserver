#ifndef DATABASE_WORKER_POOL_MYSQL_H_
#define DATABASE_WORKER_POOL_MYSQL_H_

#include "Database.h"
#include "DatabaseConnectionMySQL.h"
#include "QueryResultMySQL.h"
#include "Log.h"

#include <vector>

class DatabaseWorkerPoolMySQL : public Database
{
public:
    DatabaseWorkerPoolMySQL() : _asyncQueue(new DatabaseWorkQueueMySQL()) { }
    ~DatabaseWorkerPoolMySQL()
    {
        delete _asyncQueue;
    }
    
    bool Open(MySQLConnectionInfo connInfo, uint8_t syncThreads, uint8_t asyncThreads)
    {
        bool res = true;
        _connectionInfo = connInfo;
        
        sLog.Info(LOG_DATABASE, "Opening MySQL Database Pool '%s'. Asynchronous threads: %u, synchronous threads: %u.", _connectionInfo.DB.c_str(), asyncThreads, syncThreads);
        
        for (uint8_t i = 0; i < syncThreads; ++i)
        {
            DatabaseConnectionMySQL* conn = new DatabaseConnectionMySQL(_connectionInfo);
            res &= conn->Open();
            _connections[MYSQL_CONN_SYNC].push_back(conn);
        }
        
        for (uint8_t i = 0; i < syncThreads; ++i)
        {
            DatabaseConnectionMySQL* conn = new DatabaseConnectionMySQL(_connectionInfo, _asyncQueue);
            res &= conn->Open();
            _connections[MYSQL_CONN_ASYNC].push_back(conn);
        }
        
        if (res)
            sLog.Info(LOG_DATABASE, "MySQL Database Pool '%s' opened successfully. %u total connections.", _connectionInfo.DB.c_str(), _connections[MYSQL_CONN_SYNC].size()+_connections[MYSQL_CONN_ASYNC].size());
        else
            sLog.Error(LOG_DATABASE, "Failed opening MySQL Database Pool to '%s'.", _connectionInfo.DB.c_str());
        
        return res;
    }
    
    void Close()
    {
        sLog.Info(LOG_DATABASE, "Closing MySQL Database Pool '%s'.", _connectionInfo.DB.c_str());
        
        // Stop worker threads
        _asyncQueue->Stop();
        
        for (uint8_t i = 0; i < _connections[MYSQL_CONN_SYNC].size(); ++i)
        {
            DatabaseConnectionMySQL* conn = _connections[MYSQL_CONN_SYNC][i];
            conn->Close();
        }
        
        for (uint8_t i = 0; i < _connections[MYSQL_CONN_ASYNC].size(); ++i)
        {
            DatabaseConnectionMySQL* conn = _connections[MYSQL_CONN_ASYNC][i];
            conn->Close();
        }
        
        sLog.Info(LOG_DATABASE, "Closed all connections to MySQL Database Pool '%s'.", _connectionInfo.DB.c_str());
    }
    
    // Queries
	bool Execute(const char* query)
    {
        DatabaseConnectionMySQL* conn = GetSyncConnection();
        bool result = conn->Execute(query);
        conn->Unlock();
        return result;
    }
	ResultSetMySQL* Query(const char* query)
    {
        DatabaseConnectionMySQL* conn = GetSyncConnection();
        ResultSetMySQL* result = conn->Query(query);
        conn->Unlock();
        return result;
    }
    
    // Stmt
    bool Execute(PreparedStatement* stmt)
    {
        DatabaseConnectionMySQL* conn = GetSyncConnection();
        bool result = conn->Execute(stmt);
        conn->Unlock();
        return result;
    }
	ResultSetMySQL* Query(PreparedStatement* stmt)
    {
        DatabaseConnectionMySQL* conn = GetSyncConnection();
        ResultSetMySQL* result = conn->Query(stmt);
        conn->Unlock();
        return result;
    }
    
    // Async
    bool ExecuteAsync(const char* query)
    {
        DatabaseQueryOperationMySQL* op = new DatabaseQueryOperationMySQL(query);
        _asyncQueue->Enqueue(op);
        return true;
    }
    bool ExecuteAsync(PreparedStatement* stmt)
    {
        DatabasePreparedStatementOperationMySQL* op = new DatabasePreparedStatementOperationMySQL(stmt);
        _asyncQueue->Enqueue(op);
        return true;
    }
	bool QueryAsync(DatabaseCallback callback, const char* query)
    {
        DatabaseQueryOperationMySQL* op = new DatabaseQueryOperationMySQL(query, callback);
        _asyncQueue->Enqueue(op);
        return true;
    }
	bool QueryAsync(DatabaseCallback callback, PreparedStatement* stmt)
    {
        DatabasePreparedStatementOperationMySQL* op = new DatabasePreparedStatementOperationMySQL(stmt, callback);
        _asyncQueue->Enqueue(op);
        return true;
    }
    
	// Prepared Statements
	PreparedStatement* GetPreparedStatement(uint32_t stmtid)
    {
        return NULL;//new PreparedStatement(stmtid);
    }
    
private:
    DatabaseConnectionMySQL* GetSyncConnection()
    {
        uint32_t i;
        uint8_t conn_size = _connections[MYSQL_CONN_SYNC].size();
        DatabaseConnectionMySQL* conn = NULL;
        
        // Block until we find a free connection
        for (;;)
        {
            conn = _connections[MYSQL_CONN_SYNC][++i % conn_size];
            if (conn->LockIfReady())
                break;
        }
        
        return conn;
    }
    
    std::vector<DatabaseConnectionMySQL*> _connections[MYSQL_CONN_SIZE];
    MySQLConnectionInfo _connectionInfo;
    DatabaseWorkQueueMySQL* _asyncQueue;
};

#endif