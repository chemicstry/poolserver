#ifndef DATABASE_OPERATION_MYSQL_H_
#define DATABASE_OPERATION_MYSQL_H_

#include "Util.h"
#include "DatabaseCallback.h"
#include "PreparedStatement.h"
#include <boost/bind.hpp>

class DatabaseConnectionMySQL;

class DatabaseOperationMySQL
{
public:
    DatabaseOperationMySQL(): _conn(NULL) {}
    virtual void Execute() = 0;
    void SetConnection(DatabaseConnectionMySQL* conn) { _conn = conn; }
protected:
    DatabaseConnectionMySQL* _conn;
};

class DatabasePingOperationMySQL : public DatabaseOperationMySQL
{
    void Execute();
};

class DatabasePreparedStatementOperationMySQL : public DatabaseOperationMySQL
{
public:
    DatabasePreparedStatementOperationMySQL(PreparedStatement* stmt, DatabaseCallback callback = NULL): DatabaseOperationMySQL(), _stmt(stmt), _callback(callback) {};
    
    void Execute();
    
private:
    DatabaseCallback _callback;
    PreparedStatement* _stmt;
};

class DatabaseQueryOperationMySQL : public DatabaseOperationMySQL
{
public:
    DatabaseQueryOperationMySQL(const char* query, DatabaseCallback callback = NULL): DatabaseOperationMySQL(), _query(query), _callback(callback) {};
    
    void Execute();

private:
    DatabaseCallback _callback;
    const char* _query;
};

typedef Util::SynchronisedQueue<DatabaseOperationMySQL*> DatabaseWorkQueueMySQL;

#endif