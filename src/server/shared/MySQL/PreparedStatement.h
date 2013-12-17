#ifndef PREPARED_STATEMENT_MYSQL_H_
#define PREPARED_STATEMENT_MYSQL_H_

#include <boost/cstdint.hpp>
#include <mysql.h>

namespace MySQL
{
    enum PreparedStatementFlags
    {
        STMT_SYNC,
        STMT_ASYNC,
        STMT_BOTH = STMT_SYNC | STMT_ASYNC
    };
    
    class PreparedStatement
    {
    public:
        PreparedStatement(MYSQL_STMT* stmt);
        ~PreparedStatement();
        
        template<class T>
        void Set(const uint8_t index, const T value);
        
        void ClearParameters();
    private:
        MYSQL_STMT* _stmt;
        MYSQL_BIND* _bind;
        uint8_t _paramCount;
    };
}

#endif
