#ifndef SERVER_DATABASE_ENV_H_
#define SERVER_DATABASE_ENV_H_

#include "DatabaseWorkerPool.h"

class ServerDatabaseWorkerPoolMySQL : public MySQL::DatabaseWorkerPool
{
};

extern ServerDatabaseWorkerPoolMySQL sDatabase;

#endif
