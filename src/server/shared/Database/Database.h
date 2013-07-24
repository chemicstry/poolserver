#ifndef DATABASE_H_
#define DATABASE_H_

#include "QueryResult.h"
#include "PreparedStatement.h"
#include "DatabaseCallback.h"
#include <boost/thread.hpp>

class Database
{
public:
	// Queries
	virtual bool Execute(const char* query) = 0;
	virtual ResultSet* Query(const char* query) = 0;
    
    // Stmt
    virtual bool Execute(PreparedStatement* stmt) = 0;
	virtual ResultSet* Query(PreparedStatement* stmt) = 0;
    
    // Async
    virtual bool ExecuteAsync(const char* query) = 0;
    virtual bool ExecuteAsync(PreparedStatement* stmt) = 0;
	virtual bool QueryAsync(DatabaseCallback callback, const char* query) = 0;
	virtual bool QueryAsync(DatabaseCallback callback, PreparedStatement* stmt) = 0;
    
	// Prepared Statements
	virtual PreparedStatement* GetPreparedStatement(uint32_t index) = 0;
};

#endif
