#ifndef QUERY_RESULT_MYSQL_H_
#define QUERY_RESULT_MYSQL_H_

#include "Field.h"
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <mysql.h>

namespace MySQL
{

    class ResultSet
    {
    public:
        ResultSet(MYSQL_RES* result, MYSQL_FIELD* fields, uint64_t rowCount, uint32_t fieldCount);
        ~ResultSet();
        
        // Metadata
        uint64_t GetRowCount()
        {
            return _rowCount;
        }
        
        uint32_t GetFieldCount()
        {
            return _fieldCount;
        }
        
        bool NextRow();
        
        Field* Fetch()
        {
            return _currentRow;
        }
        
    private:
        uint64_t _rowCount;
        Field* _currentRow;
        uint32_t _fieldCount;
        void CleanUp();
        MYSQL_RES* _result;
        MYSQL_FIELD* _fields;
    };

    typedef boost::shared_ptr<ResultSet> QueryResult;

}

#endif
