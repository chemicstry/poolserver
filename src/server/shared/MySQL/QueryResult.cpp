#include "QueryResult.h"
#include "Log.h"

namespace MySQL
{
    ResultSet::ResultSet(MYSQL_RES* result, MYSQL_FIELD* fields, uint64_t rowCount, uint32_t fieldCount) :
    _result(result), _fields(fields), _rowCount(rowCount), _fieldCount(fieldCount)
    {
        _currentRow = new Field[_fieldCount];
    }

    ResultSet::~ResultSet()
    {
    }

    bool ResultSet::NextRow()
    {
        MYSQL_ROW row;

        if (!_result) {
            sLog.Debug(LOG_DATABASE, "QueryResultMySQL::NextRow(): Empty result");
            return false;
        }

        row = mysql_fetch_row(_result);
        if (!row)
        {
            sLog.Debug(LOG_DATABASE, "QueryResultMySQL::NextRow(): End of result");
            CleanUp();
            return false;
        }

        for (uint32_t i = 0; i < _fieldCount; i++)
            _currentRow[i].SetValue(row[i], _fields[i].type);

        return true;
    }

    void ResultSet::CleanUp()
    {
        if (_currentRow)
        {
            delete [] _currentRow;
            _currentRow = NULL;
        }

        if (_result)
        {
            mysql_free_result(_result);
            _result = NULL;
        }
    }
}
