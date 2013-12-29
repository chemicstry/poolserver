#include "QueryResult.h"
#include "Log.h"

namespace MySQL
{
    // Normal Query
    ResultSet::ResultSet(MYSQL_RES* result, MYSQL_FIELD* resultFields, uint64 rowCount, uint32 fieldCount) :
    _rowCount(rowCount), _fieldCount(fieldCount), _currentRow(0)
    {
        MYSQL_ROW row;
        
        while (row = mysql_fetch_row(result))
        {
            Field* fields = new Field[_fieldCount];
            
            for (uint32 i = 0; i < _fieldCount; ++i)
                fields[i].SetValue(row[i], resultFields[i].type);
            
            _rows.push_back(fields);
        }
        
        // We have it locally now!
        mysql_free_result(result);
    }
    
    // Prepared statement query
    ResultSet::ResultSet(MYSQL_RES* result, MYSQL_STMT* stmt, uint32 fieldCount) :
    _fieldCount(fieldCount), _currentRow(0)
    {
        if (!result)
            return;
        
        // Store entire result set locally from server
        if (mysql_stmt_store_result(stmt)) {
            sLog.Error(LOG_DATABASE, "mysql_stmt_store_result, cannot bind result from MySQL server. Error: %s", mysql_stmt_error(stmt));
            return;
        }
        
        // This is where we will store data
        MYSQL_BIND* bind = new MYSQL_BIND[fieldCount];
        my_bool* isNull = new my_bool[fieldCount];
        unsigned long* length = new unsigned long[fieldCount];

        // Reset
        memset(bind, 0, sizeof(MYSQL_BIND) * fieldCount);
        memset(isNull, 0, sizeof(my_bool) * fieldCount);
        memset(length, 0, sizeof(unsigned long) * fieldCount);

        // Prepare result buffer based on metadata
        uint32 i = 0;
        while (MYSQL_FIELD* field = mysql_fetch_field(result)) {
            size_t size = Field::SizeForType(field);
            
            bind[i].buffer_type = field->type;
            bind[i].buffer = malloc(size);
            memset(bind[i].buffer, 0, size);
            bind[i].buffer_length = size;
            bind[i].length = &length[i];
            bind[i].is_null = &isNull[i];
            bind[i].error = NULL;
            bind[i].is_unsigned = field->flags & UNSIGNED_FLAG;

            ++i;
        }

        // Bind result buffer to the statement
        if (mysql_stmt_bind_result(stmt, bind)) {
            sLog.Error(LOG_DATABASE, "mysql_stmt_bind_result, cannot bind result from MySQL server. Error: %s", mysql_stmt_error(stmt));
            delete[] bind;
            delete[] isNull;
            delete[] length;
            return;
        }
        
        _rowCount = mysql_stmt_num_rows(stmt);
        
        while (_NextSTMTRow(stmt))
        {
            Field* fields = new Field[fieldCount];
            
            for (uint64 fIndex = 0; fIndex < fieldCount; ++fIndex)
            {
                if (!*bind[fIndex].is_null)
                    fields[fIndex].SetByteValue(bind[fIndex].buffer, bind[fIndex].buffer_length, bind[fIndex].buffer_type, *bind[fIndex].length);
                else {
                    switch (bind[fIndex].buffer_type)
                    {
                        case MYSQL_TYPE_TINY_BLOB:
                        case MYSQL_TYPE_MEDIUM_BLOB:
                        case MYSQL_TYPE_LONG_BLOB:
                        case MYSQL_TYPE_BLOB:
                        case MYSQL_TYPE_STRING:
                        case MYSQL_TYPE_VAR_STRING:
                            fields[fIndex].SetByteValue("", bind[fIndex].buffer_length, bind[fIndex].buffer_type, *bind[fIndex].length);
                            break;
                        default:
                            fields[fIndex].SetByteValue(0, bind[fIndex].buffer_length, bind[fIndex].buffer_type, *bind[fIndex].length);
                    }
                }
            }
            
            _rows.push_back(fields);
        }
        
        // Free everything
        mysql_free_result(result);
        
        for (uint32 i = 0; i < fieldCount; ++i)
            delete (char *)bind[i].buffer;
        
        mysql_stmt_free_result(stmt);
        
        delete[] bind;
        delete[] isNull;
        delete[] length;
    }
    
    bool ResultSet::_NextSTMTRow(MYSQL_STMT* stmt)
    {
        uint8 ret = mysql_stmt_fetch(stmt);
        
        if (!ret || ret == MYSQL_DATA_TRUNCATED)
            return true;
        
        if (ret == MYSQL_NO_DATA)
            return false;
        
        sLog.Error(LOG_DATABASE, "mysql_stmt_fetch, cannot fetch result row. Error: %s", mysql_stmt_error(stmt));
        return false;
    }

    ResultSet::~ResultSet()
    {
        for (uint32 i = 0; i < _rowCount; ++i)
            delete[] _rows[i];
    }
}
