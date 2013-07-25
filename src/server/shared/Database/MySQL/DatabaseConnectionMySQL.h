#ifndef DATABASE_CONNECTION_MYSQL_H_
#define DATABASE_CONNECTION_MYSQL_H_

#include "DatabaseOperationMySQL.h"
#include "DatabaseWorkerMySQL.h"
#include "QueryResultMySQL.h"
#include "PreparedStatementMySQL.h"

#include <boost/thread.hpp>
#include <mysql.h>

enum MySQLConnectionType
{
    MYSQL_CONN_SYNC,
    MYSQL_CONN_ASYNC,
    MYSQL_CONN_SIZE
};

struct MySQLConnectionInfo
{
    std::string Host;
    uint16_t Port;
    std::string User;
    std::string Pass;
    std::string DB;
};

class DatabaseConnectionMySQL
{
public:
    DatabaseConnectionMySQL(MySQLConnectionInfo connInfo, DatabaseWorkQueueMySQL* asyncQueue = NULL);
    ~DatabaseConnectionMySQL();
    
    bool Open();
    void Close();
    
    // Ping!
    void Ping()
    {
        mysql_ping(_mysql);
    }
    
    // Queries
    bool Execute(const char* query);
    ResultSetMySQL* Query(const char* query);
    
    // Stmt
    bool Execute(PreparedStatement* stmt);
    ResultSetMySQL* Query(PreparedStatement* stmt);
    
    // Locking
    bool LockIfReady()
    {
        return _mutex.try_lock();
    }
    
    void Unlock()
    {
        _mutex.unlock();
    }
    
    MySQLConnectionType Type;
    
    void PrepareStatement(uint32_t index, const char* query);
    
private:
    bool _Query(const char *sql, MYSQL_RES* result, MYSQL_FIELD* fields, uint64_t& pRowCount, uint32_t& pFieldCount);
    
    bool _HandleMySQLErrno(uint32_t lErrno);
    
    boost::mutex _mutex;
    MYSQL* _mysql;
    DatabaseWorkQueueMySQL* _asyncQueue;
    DatabaseWorkerMySQL* _worker;
    std::vector<PreparedStatementMySQL*> _stmts;
    MySQLConnectionInfo _connectionInfo;
};

#endif
