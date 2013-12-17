#ifndef DATABASE_WORKER_MYSQL_H_
#define DATABASE_WORKER_MYSQL_H_

#include "Util.h"
#include "DatabaseOperation.h"

namespace MySQL
{
    class DatabaseConnection;

    class DatabaseWorker : public Util::GenericWorker
    {
    public:
        DatabaseWorker(DatabaseWorkQueue* asyncQueue, DatabaseConnection* conn);
        void Work();
    private:
        DatabaseWorkQueue* _asyncQueue;
        DatabaseConnection* _conn;
    };
}

#endif
