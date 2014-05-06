#include "DatabaseConnection.h"
#include "Log.h"
#include "Util.h"

namespace MySQL
{
    DatabaseConnection::DatabaseConnection(ConnectionInfo connInfo, DatabaseWorkQueue* asyncQueue) :
    _mysql(NULL), _asyncQueue(asyncQueue), _worker(NULL), _connectionInfo(connInfo)
    {
        if (_asyncQueue) {
            _worker = new DatabaseWorker(_asyncQueue, this);
            Type = MYSQL_CONN_ASYNC;
        } else
            Type = MYSQL_CONN_SYNC;
    }

    DatabaseConnection::~DatabaseConnection()
    {
        assert(_mysql);

        for (uint32_t i = 0; i < _stmts.size(); ++i)
            delete _stmts[i];

        mysql_close(_mysql);
    }

    bool DatabaseConnection::Open()
    {
        MYSQL* mysqlInit = NULL;
        mysqlInit = mysql_init(NULL);
        if (!mysqlInit)
            throw ConnectionException(Util::FS("Could not initialize Mysql connection to database `%s`", _connectionInfo.DB.c_str()));

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
            const char* error = mysql_error(mysqlInit);
            _mysqlConnectError = mysql_errno(mysqlInit); // Used for DatabaseConnection::_HandleMySQLErrno
            mysql_close(mysqlInit);
            mysqlInit = NULL;
            
            throw ConnectionException(Util::FS("Could not connect to MySQL database at %s: %s", _connectionInfo.Host.c_str(), error));
        }
    }

    void DatabaseConnection::Close()
    {
        delete this;
    }

    bool DatabaseConnection::Execute(const char* query)
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

    ResultSet* DatabaseConnection::Query(const char* query)
    {
        sLog.Debug(LOG_DATABASE, "DatabaseConnectionMySQL::Query(): %s", query);
        
        if (!query)
            return NULL;

        MYSQL_RES *result = NULL;
        MYSQL_FIELD *fields = NULL;
        uint64_t rowCount = 0;
        uint32_t fieldCount = 0;
        
        if (!_Query(query, &result, &fields, rowCount, fieldCount))
            return NULL;

        return new ResultSet(result, fields, rowCount, fieldCount);
    }

    bool DatabaseConnection::_Query(const char *query, MYSQL_RES** result, MYSQL_FIELD** fields, uint64_t& rowCount, uint32_t& fieldCount)
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

        *result = mysql_store_result(_mysql);
        rowCount = mysql_affected_rows(_mysql);
        fieldCount = mysql_field_count(_mysql);

        if (!*result)
            return false;

        if (!rowCount) {
            mysql_free_result(*result);
            return false;
        }

        *fields = mysql_fetch_fields(*result);
        
        return true;
    }
    
    bool DatabaseConnection::_Query(PreparedStatement* stmt, MYSQL_RES** result, MYSQL_STMT** resultSTMT, uint32& fieldCount)
    {
        if (!_mysql)
            return false;
        
        ConnectionPreparedStatement* cstmt = GetPreparedStatement(stmt->_index);
        
        if (!cstmt) {
            sLog.Error(LOG_DATABASE, "STMT id: %u not found!", stmt->_index);
            return false;
        }
        
        cstmt->BindParameters(stmt);
        
        MYSQL_STMT* mSTMT = cstmt->GetSTMT();
        MYSQL_BIND* mBIND = cstmt->GetBind();
        
        if (mysql_stmt_bind_param(mSTMT, mBIND))
        {
            uint32 lErrno = mysql_errno(_mysql);
            sLog.Error(LOG_DATABASE, "STMT Execute Error[%u]: %s", lErrno, mysql_stmt_error(mSTMT));

            if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
                return Execute(stmt);       // Try again

            cstmt->ClearParameters();
            return false;
        }

        if (mysql_stmt_execute(mSTMT))
        {
            uint32 lErrno = mysql_errno(_mysql);
            sLog.Error(LOG_DATABASE, "STMT Execute Error[%u]: %s", lErrno, mysql_stmt_error(mSTMT));

            if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
                return _Query(stmt, result, resultSTMT, fieldCount);       // Try again

            cstmt->ClearParameters();
            return false;
        }

        cstmt->ClearParameters();
        
        *result = mysql_stmt_result_metadata(mSTMT);
        fieldCount = mysql_stmt_field_count(mSTMT);
        *resultSTMT = mSTMT;
        
        return true;
    }

    bool DatabaseConnection::Execute(PreparedStatement* stmt)
    {
        if (!_mysql)
            return false;
        
        ConnectionPreparedStatement* cstmt = GetPreparedStatement(stmt->_index);
        
        if (!cstmt) {
            sLog.Error(LOG_DATABASE, "STMT id: %u not found!", stmt->_index);
            return false;
        }
        
        cstmt->BindParameters(stmt);
        
        MYSQL_STMT* mSTMT = cstmt->GetSTMT();
        MYSQL_BIND* mBIND = cstmt->GetBind();
        
        if (mysql_stmt_bind_param(mSTMT, mBIND))
        {
            uint32 lErrno = mysql_errno(_mysql);
            sLog.Error(LOG_DATABASE, "STMT Execute Error[%u]: %s", lErrno, mysql_stmt_error(mSTMT));

            if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
                return Execute(stmt);       // Try again

            cstmt->ClearParameters();
            return false;
        }

        if (mysql_stmt_execute(mSTMT))
        {
            uint32 lErrno = mysql_errno(_mysql);
            sLog.Error(LOG_DATABASE, "STMT Execute Error[%u]: %s", lErrno, mysql_stmt_error(mSTMT));

            if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
                return Execute(stmt);       // Try again

            cstmt->ClearParameters();
            return false;
        }

        cstmt->ClearParameters();
        
        return true;
    }

    ResultSet* DatabaseConnection::Query(PreparedStatement* stmt)
    {
        MYSQL_RES* result = NULL;
        MYSQL_STMT* resultSTMT = NULL;
        uint32 fieldCount = 0;

        if (!_Query(stmt, &result, &resultSTMT, fieldCount))
            return NULL;

        if (mysql_more_results(_mysql))
            mysql_next_result(_mysql);
        
        return new ResultSet(result, resultSTMT, fieldCount);
    }
    
    bool DatabaseConnection::PrepareStatement(uint32 index, const char* sql)
    {
        if (!_mysql)
            return false;
        
        // For reconnection case
        //if (m_reconnecting)
        //    delete m_stmts[index];

        MYSQL_STMT* stmt = mysql_stmt_init(_mysql);
        
        if (!stmt) {
            sLog.Error(LOG_DATABASE, "In mysql_stmt_init() id: %u, sql: \"%s\"", index, sql);
            sLog.Error(LOG_DATABASE, "%s", mysql_error(_mysql));
            return false;
        }
        
        if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
            sLog.Error(LOG_DATABASE, "In mysql_stmt_init() id: %u, sql: \"%s\"", index, sql);
            sLog.Error(LOG_DATABASE, "%s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }
        
        // Set flags to update max_length property
        my_bool mysql_c_api_sucks = true;
        mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void*)&mysql_c_api_sucks);
        
        // Resize stmt vector
        if (index >= _stmts.size())
            _stmts.resize(index+1);
        
        ConnectionPreparedStatement* cstmt = new ConnectionPreparedStatement(stmt);
        _stmts[index] = cstmt;
        
        sLog.Debug(LOG_DATABASE, "Prepared STMT id: %u, sql: \"%s\"", index, sql);
        
        return true;
    }

    bool DatabaseConnection::_HandleMySQLErrno(uint32 lErrno)
    {
        switch (lErrno)
        {
            case CR_SERVER_GONE_ERROR:
            case CR_SERVER_LOST:
            case CR_INVALID_CONN_HANDLE:
            case CR_SERVER_LOST_EXTENDED:
            {
                mysql_close(_mysql);
                
                try {
                    Open();
                    return true;
                } catch(std::exception& e) {
                    sLog.Error(LOG_DATABASE, "Failed to connect to mysql database: %s", e.what());
                    
                    // It's possible this attempted reconnect throws 2006 at us. To prevent crazy recursive calls, sleep here.
                    boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
                    return _HandleMySQLErrno(_mysqlConnectError);
                }
            }
            default:
                sLog.Error(LOG_DATABASE, "Unhandled MySQL errno %u. Unexpected behaviour possible.", lErrno);
                return false;
        }
    }
}
