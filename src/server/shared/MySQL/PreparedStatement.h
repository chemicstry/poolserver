#ifndef PREPARED_STATEMENT_MYSQL_H_
#define PREPARED_STATEMENT_MYSQL_H_

#include "Common.h"

#include <boost/variant.hpp>
#include <mysql.h>
#include <vector>

namespace MySQL
{
    enum PreparedStatementFlags
    {
        STMT_SYNC       = 1,
        STMT_ASYNC      = 2,
        STMT_BOTH       = STMT_SYNC | STMT_ASYNC
    };
    
    typedef boost::variant<int8, int16, int32, int64, uint8, uint16, uint32, uint64, float, double, std::string> MySQLValue;
    
    enum MySQLValueTypes
    {
        MYSQL_UINT8,
        MYSQL_UINT16,
        MYSQL_UINT32,
        MYSQL_UINT64,
        MYSQL_INT8,
        MYSQL_INT16,
        MYSQL_INT32,
        MYSQL_INT64,
        MYSQL_FLOAT,
        MYSQL_DOUBLE,
        MYSQL_STRING,
        MYSQL_NULL
    };
    
    struct PreparedStatementData
    {
        MySQLValue value;
        MySQLValueTypes type;
    };
    
    // High level stmt
    class PreparedStatement
    {
        friend class ConnectionPreparedStatement;
        friend class DatabaseConnection;
    
    public:
        PreparedStatement(uint32 index)
        {
            _index = index;
        }
        
        template<class T>
        void Set(const uint8 index, const T value, const MySQLValueTypes type)
        {
            if (index >= data.size())
                data.resize(index+1);
            
            data[index].value = value;
            data[index].type = type;
        }
        
        void SetBool(const uint8 index, const bool value) {
            SetUInt8(index, value ? 1 : 0);
        }
        void SetUInt8(const uint8 index, const uint8 value) {
            Set<uint8>(index, value, MYSQL_UINT8);
        }
        void SetUInt16(const uint8 index, const uint16 value) {
            Set<uint16>(index, value, MYSQL_UINT16);
        }
        void SetUInt32(const uint8 index, const uint32 value) {
            Set<uint32>(index, value, MYSQL_UINT32);
        }
        void SetUInt64(const uint8 index, const uint64 value) {
            Set<uint64>(index, value, MYSQL_UINT64);
        }
        void SetInt8(const uint8 index, const int8 value) {
            Set<int8>(index, value, MYSQL_INT8);
        }
        void SetInt16(const uint8 index, const int16 value) {
            Set<int16>(index, value, MYSQL_INT16);
        }
        void SetInt32(const uint8 index, const int32 value) {
            Set<int32>(index, value, MYSQL_INT32);
        }
        void SetInt64(const uint8 index, const int64 value) {
            Set<int64>(index, value, MYSQL_INT64);
        }
        void SetFloat(const uint8 index, const float value) {
            Set<float>(index, value, MYSQL_FLOAT);
        }
        void SetDouble(const uint8 index, const double value) {
            Set<double>(index, value, MYSQL_DOUBLE);
        }
        void SetString(const uint8 index, const std::string& value) {
            Set<std::string>(index, value, MYSQL_STRING);
        }
        void SetNull(const uint8 index) {
            Set<int64>(index, 0, MYSQL_NULL);
        }
        
    protected:
        std::vector<PreparedStatementData> data;
        uint32 _index;
    };
    
    // Connection specific stmt
    class ConnectionPreparedStatement
    {
    public:
        ConnectionPreparedStatement(MYSQL_STMT* stmt);
        ~ConnectionPreparedStatement();
        
        /*template<class T>
        void Set(const uint8_t index, const T value);*/
        void BindParameters(PreparedStatement* stmt);
        
        void SetValue(uint8 index, enum_field_types type, const void* value, uint32 len, bool isUnsigned);
        void SetString(uint8 index, std::string str);
        void SetNull(uint8 index);
        
        MYSQL_STMT* GetSTMT()
        {
            return _stmt;
        }
        
        MYSQL_BIND* GetBind()
        {
            return _bind;
        }
        
        void ClearParameters();
    private:
        MYSQL_STMT* _stmt;
        MYSQL_BIND* _bind;
        uint8_t _paramCount;
    };
}

#endif
