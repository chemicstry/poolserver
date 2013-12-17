#ifndef DATABASE_WORKER_POOL_MYSQL_H_
#define DATABASE_WORKER_POOL_MYSQL_H_

#include "DatabaseConnection.h"
#include "PreparedStatement.h"
#include "QueryResult.h"
#include "Log.h"

#include <vector>

namespace MySQL
{
    class DatabaseWorkerPool
    {
    public:
        DatabaseWorkerPool() : _asyncQueue(new DatabaseWorkQueue()) { }
        ~DatabaseWorkerPool()
        {
            delete _asyncQueue;
        }
        
        bool Open(ConnectionInfo connInfo, uint8_t syncThreads, uint8_t asyncThreads);
        
        void Close();
        
        // Queries
        bool Execute(const char* query)
        {
            DatabaseConnection* conn = GetSyncConnection();
            bool result = conn->Execute(query);
            conn->Unlock();
            return result;
        }
        QueryResult Query(const char* query)
        {
            DatabaseConnection* conn = GetSyncConnection();
            ResultSet* result = conn->Query(query);
            conn->Unlock();
            return QueryResult(result);
        }
        
        // Stmt
        bool Execute(PreparedStatement* stmt)
        {
            DatabaseConnection* conn = GetSyncConnection();
            bool result = conn->Execute(stmt);
            conn->Unlock();
            return result;
        }
        QueryResult Query(PreparedStatement* stmt)
        {
            DatabaseConnection* conn = GetSyncConnection();
            ResultSet* result = conn->Query(stmt);
            conn->Unlock();
            return QueryResult(result);
        }
        
        // Async
        bool ExecuteAsync(const char* query)
        {
            DatabaseQueryOperation* op = new DatabaseQueryOperation(query);
            _asyncQueue->Enqueue(op);
            return true;
        }
        bool ExecuteAsync(PreparedStatement* stmt)
        {
            DatabasePreparedStatementOperation* op = new DatabasePreparedStatementOperation(stmt);
            _asyncQueue->Enqueue(op);
            return true;
        }
        bool QueryAsync(const char* query, DatabaseCallback callback)
        {
            DatabaseQueryOperation* op = new DatabaseQueryOperation(query, callback);
            _asyncQueue->Enqueue(op);
            return true;
        }
        bool QueryAsync(PreparedStatement* stmt, DatabaseCallback callback )
        {
            DatabasePreparedStatementOperation* op = new DatabasePreparedStatementOperation(stmt, callback);
            _asyncQueue->Enqueue(op);
            return true;
        }
        
        bool PrepareStatement(index, const char* sql, PreparedStatementFlags flags);
        
        // Prepared Statements
        PreparedStatement* GetPreparedStatement(uint32_t stmtid)
        {
            return NULL;//new PreparedStatement(stmtid);
        }
        
    private:
        DatabaseConnection* GetSyncConnection()
        {
            uint32_t i;
            uint8_t conn_size = _connections[MYSQL_CONN_SYNC].size();
            DatabaseConnection* conn = NULL;
            
            // Block until we find a free connection
            for (;;)
            {
                conn = _connections[MYSQL_CONN_SYNC][++i % conn_size];
                if (conn->LockIfReady())
                    break;
            }
            
            return conn;
        }
        
        std::vector<DatabaseConnection*> _connections[MYSQL_CONN_SIZE];
        ConnectionInfo _connectionInfo;
        DatabaseWorkQueue* _asyncQueue;
    };
}

#endif
