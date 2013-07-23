#ifndef DATABASE_H
#define DATABASE_H

class Database;
class ResultSet;
class Fields;
class PreparedStatement;

enum PreparedStatementEnum
{
	STMT_INSERT_SHARE               = 1,
};

class Database
{
public:
	
	// Ping!
	virtual Ping();
	
	// Queries
	virtual void Execute(const char* query);
	virtual void Execute(PreparedStatement* stmt);
	virtual ResultSet* Query(const char* query);
	virtual ResultSet* Query(PreparedStatement* stmt);
	
	// Prepared Statements
	virtual PreparedStatement* GetPreparedStatement(PreparedStatementEnum smtid);
	
	
};

#endif