#include "PreparedStatementMySQL.h"
#include <cstring>

PreparedStatementMySQL::PreparedStatementMySQL(MYSQL_STMT* stmt) :
_stmt(stmt), _bind(NULL), PreparedStatement(0)
{
    _paramCount = mysql_stmt_param_count(stmt);
    _bind = new MYSQL_BIND[_paramCount];
    memset(_bind, 0, sizeof(MYSQL_BIND)*_paramCount);
}

PreparedStatementMySQL::~PreparedStatementMySQL()
{
    ClearParameters();
    if (_stmt->bind_result_done)
    {
        delete[] _stmt->bind->length;
        delete[] _stmt->bind->is_null;
    }
    mysql_stmt_close(_stmt);
    delete[] _bind;
    this->~PreparedStatement();
}

void PreparedStatementMySQL::ClearParameters()
{
    for (uint8_t i = 0; i < _paramCount; ++i)
    {
        delete _bind[i].length;
        _bind[i].length = NULL;
        delete[] (char*) _bind[i].buffer;
        _bind[i].buffer = NULL;
    }
}