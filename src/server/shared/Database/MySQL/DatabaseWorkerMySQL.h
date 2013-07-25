#ifndef DATABASE_WORKER_MYSQL_H_
#define DATABASE_WORKER_MYSQL_H_

#include "Util.h"
#include "DatabaseOperationMySQL.h"

class DatabaseConnectionMySQL;

class DatabaseWorkerMySQL : public Util::GenericWorker
{
public:
    DatabaseWorkerMySQL(DatabaseWorkQueueMySQL* asyncQueue, DatabaseConnectionMySQL* conn);
    void Work();
private:
    DatabaseWorkQueueMySQL* _asyncQueue;
    DatabaseConnectionMySQL* _conn;
};

#endif
