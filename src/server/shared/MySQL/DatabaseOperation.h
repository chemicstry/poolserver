#ifndef DATABASE_OPERATION_MYSQL_H_
#define DATABASE_OPERATION_MYSQL_H_

#include "Util.h"
#include "DatabaseCallback.h"
#include "PreparedStatement.h"
#include <boost/bind.hpp>

namespace MySQL
{
    class DatabaseConnection;

    class DatabaseOperation
    {
    public:
        DatabaseOperation(): _conn(NULL) {}
        virtual ~DatabaseOperation() {}
        virtual void Execute() = 0;
        void SetConnection(DatabaseConnection* conn) { _conn = conn; }
    protected:
        DatabaseConnection* _conn;
    };

    class DatabasePingOperation : public DatabaseOperation
    {
        void Execute();
    };

    class DatabasePreparedStatementOperation : public DatabaseOperation
    {
    public:
        DatabasePreparedStatementOperation(PreparedStatement* stmt, DatabaseCallback callback = NULL): DatabaseOperation(), _stmt(stmt), _callback(callback) {};
        
        void Execute();
        
    private:
        DatabaseCallback _callback;
        PreparedStatement* _stmt;
    };

    class DatabaseQueryOperation : public DatabaseOperation
    {
    public:
        DatabaseQueryOperation(const char* query, DatabaseCallback callback = NULL): DatabaseOperation(), _callback(callback), _query(query) {}
        
        ~DatabaseQueryOperation() {}
        
        void Execute();

    private:
        DatabaseCallback _callback;
        std::string _query;
    };

    typedef Util::SynchronisedQueue<DatabaseOperation*> DatabaseWorkQueue;
}

#endif
