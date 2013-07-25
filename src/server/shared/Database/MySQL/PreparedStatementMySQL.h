#ifndef PREPARED_STATEMENT_MYSQL_H_
#define PREPARED_STATEMENT_MYSQL_H_

#include "PreparedStatement.h"
#include <boost/cstdint.hpp>
#include <mysql.h>

class PreparedStatementMySQL : public PreparedStatement
{
public:
    PreparedStatementMySQL(MYSQL_STMT* stmt);
    ~PreparedStatementMySQL();
    
    template<class T>
    void Set(const uint8_t index, const T value);
    
    void ClearParameters();
private:
    MYSQL_STMT* _stmt;
    MYSQL_BIND* _bind;
    uint8_t _paramCount;
};

#endif
