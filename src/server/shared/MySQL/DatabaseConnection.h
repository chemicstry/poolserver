#ifndef DATABASE_CONNECTION_MYSQL_H_
#define DATABASE_CONNECTION_MYSQL_H_

#include <boost/thread.hpp>
#include <mysql.h>
#include <string>

#include "DatabaseOperation.h"
#include "DatabaseWorker.h"
#include "QueryResult.h"
#include "PreparedStatement.h"
#include "Log.h"
#include "Exception.h"

namespace MySQL
{
    enum ClientError
    {
        CR_SERVER_GONE_ERROR        = 2006,
        CR_SERVER_LOST              = 2013,
        CR_INVALID_CONN_HANDLE      = 2048,
        CR_SERVER_LOST_EXTENDED     = 2055
    };
    
    enum ConnectionType
    {
        MYSQL_CONN_SYNC,
        MYSQL_CONN_ASYNC,
        MYSQL_CONN_SIZE
    };

    struct ConnectionInfo
    {
        std::string Host;
        uint16_t Port;
        std::string User;
        std::string Pass;
        std::string DB;
    };
    
    class ConnectionException: public Exception
    {
    public:
        ConnectionException(const char *text): Exception(text) {}
        ConnectionException(std::string text): Exception(text) {}
    };

    class DatabaseConnection
    {
    public:
        DatabaseConnection(ConnectionInfo connInfo, DatabaseWorkQueue* asyncQueue = NULL);
        ~DatabaseConnection();
        
        bool Open();
        void Close();
        
        // Ping!
        void Ping()
        {
            mysql_ping(_mysql);
        }
        
        // Queries
        bool Execute(const char* query);
        ResultSet* Query(const char* query);
        
        // Stmt
        bool Execute(PreparedStatement* stmt);
        ResultSet* Query(PreparedStatement* stmt);
        
        // Locking
        bool LockIfReady()
        {
            return _mutex.try_lock();
        }
        
        void Unlock()
        {
            _mutex.unlock();
        }
        
        ConnectionType Type;
        
        bool PrepareStatement(uint32_t index, const char* sql);
        
        ConnectionPreparedStatement* GetPreparedStatement(uint32 index)
        {
            if (index >= _stmts.size())
                return NULL;
            return _stmts[index];
        }
        
        std::string Escape(std::string text)
        {
            char* buf = new char[text.length()*2 + 1];
            mysql_real_escape_string(_mysql, buf, text.c_str(), text.length());
            std::string result(buf);
            delete[] buf;
            return result;
        }
        
    private:
        bool _Query(const char *sql, MYSQL_RES** result, MYSQL_FIELD** fields, uint64& rowCount, uint32& fieldCount);
        bool _Query(PreparedStatement* stmt, MYSQL_RES** result, MYSQL_STMT** resultSTMT, uint32& fieldCount);
        
        bool _HandleMySQLErrno(uint32_t lErrno);
        
        boost::mutex _mutex;
        MYSQL* _mysql;
        uint32 _mysqlConnectError;
        DatabaseWorkQueue* _asyncQueue;
        DatabaseWorker* _worker;
        std::vector<ConnectionPreparedStatement*> _stmts;
        ConnectionInfo _connectionInfo;
    };
}

#endif
