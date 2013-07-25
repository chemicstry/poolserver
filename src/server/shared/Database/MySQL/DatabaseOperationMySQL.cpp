#include "DatabaseOperationMySQL.h"
#include "DatabaseConnectionMySQL.h"

void DatabasePingOperationMySQL::Execute()
{
    _conn->Ping();
}

void DatabasePreparedStatementOperationMySQL::Execute()
{
    if (_callback) {
        ResultSetMySQL* result = _conn->Query(_stmt);
        _callback(result);
    } else
        _conn->Execute(_stmt);
}


void DatabaseQueryOperationMySQL::Execute()
{
    if (_callback) {
        ResultSetMySQL* result = _conn->Query(_query);
        _callback(result);
    } else
        _conn->Execute(_query);
}
