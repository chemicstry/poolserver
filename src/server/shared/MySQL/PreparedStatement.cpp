#include "PreparedStatement.h"
#include <cstring>

namespace MySQL
{
    ConnectionPreparedStatement::ConnectionPreparedStatement(MYSQL_STMT* stmt) :
    _stmt(stmt), _bind(NULL)
    {
        _paramCount = mysql_stmt_param_count(stmt);
        _bind = new MYSQL_BIND[_paramCount];
        memset(_bind, 0, sizeof(MYSQL_BIND)*_paramCount);
    }

    ConnectionPreparedStatement::~ConnectionPreparedStatement()
    {
        ClearParameters();
        if (_stmt->bind_result_done)
        {
            delete[] _stmt->bind->length;
            delete[] _stmt->bind->is_null;
        }
        mysql_stmt_close(_stmt);
        delete[] _bind;
    }
    
    void ConnectionPreparedStatement::BindParameters(PreparedStatement* stmt)
    {
        for (uint8 i = 0; i < stmt->data.size(); ++i)
        {
            switch (stmt->data[i].type)
            {
                case MYSQL_UINT8:
                    SetValue(i, MYSQL_TYPE_TINY, &boost::get<uint8>(stmt->data[i].value), sizeof(uint8), true);
                    break;
                case MYSQL_UINT16:
                    SetValue(i, MYSQL_TYPE_SHORT, &boost::get<uint16>(stmt->data[i].value), sizeof(uint16), true);
                    break;
                case MYSQL_UINT32:
                    SetValue(i, MYSQL_TYPE_LONG, &boost::get<uint32>(stmt->data[i].value), sizeof(uint32), true);
                    break;
                case MYSQL_UINT64:
                    SetValue(i, MYSQL_TYPE_LONGLONG, &boost::get<uint64>(stmt->data[i].value), sizeof(uint64), true);
                    break;
                case MYSQL_INT8:
                    SetValue(i, MYSQL_TYPE_TINY, &boost::get<int8>(stmt->data[i].value), sizeof(int8), false);
                    break;
                case MYSQL_INT16:
                    SetValue(i, MYSQL_TYPE_SHORT, &boost::get<int16>(stmt->data[i].value), sizeof(int16), false);
                    break;
                case MYSQL_INT32:
                    SetValue(i, MYSQL_TYPE_LONG, &boost::get<int32>(stmt->data[i].value), sizeof(int32), false);
                    break;
                case MYSQL_INT64:
                    SetValue(i, MYSQL_TYPE_LONGLONG, &boost::get<int64>(stmt->data[i].value), sizeof(int64), false);
                    break;
                case MYSQL_FLOAT:
                    SetValue(i, MYSQL_TYPE_FLOAT, &boost::get<float>(stmt->data[i].value), sizeof(float), false);
                    break;
                case MYSQL_DOUBLE:
                    SetValue(i, MYSQL_TYPE_DOUBLE, &boost::get<double>(stmt->data[i].value), sizeof(double), false);
                    break;
                case MYSQL_STRING:
                    SetString(i, boost::get<std::string>(stmt->data[i].value));
                    break;
                case MYSQL_NULL:
                    SetNull(i);
                    break;
                default:
                    // need assert?
                    break;
            }
        }
    }
    
    void ConnectionPreparedStatement::SetValue(uint8 index, enum_field_types type, const void* value, uint32 len, bool isUnsigned)
    {
        MYSQL_BIND* param = &_bind[index];
        
        param->buffer_type = type;
        delete[] (char *)param->buffer;
        param->buffer = new char[len];
        param->buffer_length = 0;
        param->is_null_value = 0;
        param->length = NULL;               // Only != NULL for strings
        param->is_unsigned = isUnsigned;

        memcpy(param->buffer, value, len);
    }
    
    void ConnectionPreparedStatement::SetString(uint8 index, std::string str)
    {
        MYSQL_BIND* param = &_bind[index];
        size_t len = str.size() + 1;
        param->buffer_type = MYSQL_TYPE_VAR_STRING;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = new char[len];
        param->buffer_length = len;
        param->is_null_value = 0;
        delete param->length;
        param->length = new unsigned long(len-1);
        memcpy(param->buffer, str.c_str(), len);
    }
    
    void ConnectionPreparedStatement::SetNull(uint8 index)
    {
        MYSQL_BIND* param = &_bind[index];
        param->buffer_type = MYSQL_TYPE_NULL;
        delete [] static_cast<char *>(param->buffer);
        param->buffer = NULL;
        param->buffer_length = 0;
        param->is_null_value = 1;
        delete param->length;
        param->length = NULL;
    }

    void ConnectionPreparedStatement::ClearParameters()
    {
        for (uint8 i = 0; i < _paramCount; ++i)
        {
            delete _bind[i].length;
            _bind[i].length = NULL;
            delete[] (char*) _bind[i].buffer;
            _bind[i].buffer = NULL;
        }
    }
}
