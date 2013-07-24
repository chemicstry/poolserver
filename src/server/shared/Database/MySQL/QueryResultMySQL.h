#ifndef QUERY_RESULT_MYSQL_H_
#define QUERY_RESULT_MYSQL_H_

#include "QueryResult.h"
#include "FieldMySQL.h"
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <mysql.h>

class ResultSetMySQL : public ResultSet
{
public:
    ResultSetMySQL(MYSQL_RES* result, MYSQL_FIELD* fields, uint64_t rowCount, uint32_t fieldCount);
    ~ResultSetMySQL();
    
    // Metadata
    uint64_t GetRowCount()
    {
        return _rowCount;
    }
    
    uint32_t GetFieldCount()
    {
        return _fieldCount;
    }
    
    bool NextRow() {};
    Field* Fetch() {};
    
private:
    uint64_t _rowCount;
    FieldMySQL* _currentRow;
    uint32_t _fieldCount;
    void CleanUp();
    MYSQL_RES* _result;
    MYSQL_FIELD* _fields;
};

#endif