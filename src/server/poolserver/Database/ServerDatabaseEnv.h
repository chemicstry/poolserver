#ifndef SERVER_DATABASE_ENV_H_
#define SERVER_DATABASE_ENV_H_

#include "DatabaseEnv.h"

class ServerDatabaseWorkerPoolMySQL : public DatabaseWorkerPoolMySQL
{
};

extern ServerDatabaseWorkerPoolMySQL sDatabase;

#endif
