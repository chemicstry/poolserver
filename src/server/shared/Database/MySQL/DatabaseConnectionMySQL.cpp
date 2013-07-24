#include "DatabaseConnectionMySQL.h"
#include "Log.h"

DatabaseConnectionMySQL::DatabaseConnectionMySQL(MySQLConnectionInfo connInfo, DatabaseWorkQueueMySQL* asyncQueue) :
_mysql(NULL), _asyncQueue(asyncQueue), _worker(NULL), _connectionInfo(connInfo)
{
    if (_asyncQueue) {
        _worker = new DatabaseWorkerMySQL(_asyncQueue, this);
        Type = MYSQL_CONN_ASYNC;
    } else
        Type = MYSQL_CONN_SYNC;
}

DatabaseConnectionMySQL::~DatabaseConnectionMySQL()
{
    assert(_mysql);

    for (uint32_t i = 0; i < _stmts.size(); ++i)
        delete _stmts[i];

    mysql_close(_mysql);
}

bool DatabaseConnectionMySQL::Open()
{
    MYSQL* mysqlInit;
    mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        sLog.Error(LOG_DATABASE, "Could not initialize Mysql connection to database `%s`", _connectionInfo.DB.c_str());
        return false;
    }

    mysql_options(mysqlInit, MYSQL_SET_CHARSET_NAME, "utf8");

    _mysql = mysql_real_connect(mysqlInit, _connectionInfo.Host.c_str(), _connectionInfo.User.c_str(),
        _connectionInfo.Pass.c_str(), _connectionInfo.DB.c_str(), _connectionInfo.Port, NULL, 0);

    if (_mysql)
    {
        sLog.Info(LOG_DATABASE, "Connected to MySQL database at %s", _connectionInfo.Host.c_str());
        mysql_autocommit(_mysql, 1);

        // set connection properties to UTF8 to properly handle locales for different
        // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
        mysql_set_character_set(_mysql, "utf8");
        
        return true;
    }
    else
    {
        sLog.Error(LOG_DATABASE, "Could not connect to MySQL database at %s: %s\n", _connectionInfo.Host.c_str(), mysql_error(mysqlInit));
        mysql_close(mysqlInit);
        return false;
    }
}

void DatabaseConnectionMySQL::Close()
{
    delete this;
}

bool DatabaseConnectionMySQL::Execute(const char* query)
{
    sLog.Debug(LOG_DATABASE, "DatabaseConnectionMySQL::Execute(): %s", query);
    
    if (!query || !_mysql)
        return false;

    if (mysql_query(_mysql, query))
    {
        uint32_t lErrno = mysql_errno(_mysql);

        sLog.Error(LOG_DATABASE, "[%u] %s", lErrno, mysql_error(_mysql));

        if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
            return Execute(query);       // Try again

        return false;
    }
    else
        return true;
}

ResultSetMySQL* DatabaseConnectionMySQL::Query(const char* query)
{
    sLog.Debug(LOG_DATABASE, "DatabaseConnectionMySQL::Query(): %s", query);
    
    if (!query)
        return NULL;

    MYSQL_RES *result = NULL;
    MYSQL_FIELD *fields = NULL;
    uint64_t rowCount = 0;
    uint32_t fieldCount = 0;
    
    if (!_Query(query, result, fields, rowCount, fieldCount))
        return NULL;

    return new ResultSetMySQL(result, fields, rowCount, fieldCount);
}

bool DatabaseConnectionMySQL::_Query(const char *query, MYSQL_RES* result, MYSQL_FIELD* fields, uint64_t& rowCount, uint32_t& fieldCount)
{
    if (!_mysql)
        return false;
    
    if (mysql_query(_mysql, query))
    {
        uint32_t lErrno = mysql_errno(_mysql);
        
        sLog.Error(LOG_DATABASE, "[%u] %s", lErrno, mysql_error(_mysql));

        if (_HandleMySQLErrno(lErrno))      // If it returns true, an error was handled successfully (i.e. reconnection)
            return _Query(query, result, fields, rowCount, fieldCount);    // We try again

        return false;
    }

    result = mysql_store_result(_mysql);
    rowCount = mysql_affected_rows(_mysql);
    fieldCount = mysql_field_count(_mysql);

    if (!result)
        return false;

    if (!rowCount)
    {
        mysql_free_result(result);
        return false;
    }

    fields = mysql_fetch_fields(result);

    return true;
}

bool DatabaseConnectionMySQL::Execute(PreparedStatement* stmt)
{
}

ResultSetMySQL* DatabaseConnectionMySQL::Query(PreparedStatement* stmt)
{
}

bool DatabaseConnectionMySQL::_HandleMySQLErrno(uint32_t lErrno)
{
    return false;
}