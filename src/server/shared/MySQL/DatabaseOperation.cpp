#include "DatabaseOperation.h"
#include "DatabaseConnection.h"

namespace MySQL
{
    void DatabasePingOperation::Execute()
    {
        _conn->Ping();
    }

    void DatabasePreparedStatementOperation::Execute()
    {
        if (_callback) {
            ResultSet* result = _conn->Query(_stmt);
            _callback(QueryResult(result));
        } else
            _conn->Execute(_stmt);
    }


    void DatabaseQueryOperation::Execute()
    {
        if (_callback) {
            ResultSet* result = _conn->Query(_query.c_str());
            _callback(QueryResult(result));
        } else
            _conn->Execute(_query.c_str());
    }
}
