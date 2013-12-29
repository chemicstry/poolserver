#ifndef QUERY_RESULT_MYSQL_H_
#define QUERY_RESULT_MYSQL_H_

#include "Common.h"
#include "Field.h"

#include <boost/shared_ptr.hpp>
#include <cassert>
#include <vector>
#include <mysql.h>

namespace MySQL
{

    class ResultSet
    {
    public:
        // Normal query
        ResultSet(MYSQL_RES* result, MYSQL_FIELD* resultFields, uint64 rowCount, uint32 fieldCount);
        // Prepared statement query
        ResultSet(MYSQL_RES* result, MYSQL_STMT* stmt, uint32 fieldCount);
        ~ResultSet();
        
        // Metadata
        uint64 GetRowCount()
        {
            return _rowCount;
        }
        
        uint32 GetFieldCount()
        {
            return _fieldCount;
        }
        
        Field* FetchRow()
        {
            if (_currentRow >= _rowCount)
                return NULL;
            
            return _rows[_currentRow++];
        }
        
    private:
        bool _NextSTMTRow(MYSQL_STMT* stmt);
        uint64 _rowCount;
        uint64 _currentRow;
        uint32 _fieldCount;
        std::vector<Field*> _rows;
    };

    typedef boost::shared_ptr<ResultSet> QueryResult;

}

#endif
