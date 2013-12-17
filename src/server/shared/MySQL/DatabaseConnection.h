#ifndef DATABASE_CONNECTION_MYSQL_H_
#define DATABASE_CONNECTION_MYSQL_H_

#include "DatabaseOperation.h"
#include "DatabaseWorker.h"
#include "QueryResult.h"
#include "PreparedStatement.h"

#include <boost/thread.hpp>
#include <mysql.h>

namespace MySQL
{
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
        
        void PrepareStatement(uint32_t index, const char* sql);
        
    private:
        bool _Query(const char *sql, MYSQL_RES** result, MYSQL_FIELD** fields, uint64_t& pRowCount, uint32_t& pFieldCount);
        
        bool _HandleMySQLErrno(uint32_t lErrno);
        
        boost::mutex _mutex;
        MYSQL* _mysql;
        DatabaseWorkQueue* _asyncQueue;
        DatabaseWorker* _worker;
        std::vector<PreparedStatement*> _stmts;
        ConnectionInfo _connectionInfo;
    };
}

#endif
