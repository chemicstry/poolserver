#include "DatabaseWorkerMySQL.h"
#include "Log.h"

DatabaseWorkerMySQL::DatabaseWorkerMySQL(DatabaseWorkQueueMySQL* asyncQueue, DatabaseConnectionMySQL* conn) :
_asyncQueue(asyncQueue), _conn(conn)
{
    Activate();
}

void DatabaseWorkerMySQL::Work()
{
    sLog.Debug(LOG_DATABASE, "Database worker thread started");
    
    if (!this->_asyncQueue)
        return;
    
    DatabaseOperationMySQL* op = NULL;
    for (;;)
    {
        op = this->_asyncQueue->Dequeue();
        
        if (!op) {
            sLog.Debug(LOG_DATABASE, "Database worker thread exiting...");
            return;
        }
        
        sLog.Debug(LOG_DATABASE, "Database worker working...");
        
        op->SetConnection(_conn);
        op->Execute();
        
        delete op;
        
        sLog.Debug(LOG_DATABASE, "Database worker finished!");
    }
}
