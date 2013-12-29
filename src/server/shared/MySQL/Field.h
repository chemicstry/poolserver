#ifndef FIELD_MYSQL_H_
#define FIELD_MYSQL_H_

#include "Log.h"
#include <mysql.h>
#include <boost/lexical_cast.hpp>

namespace MySQL
{
    class Field
    {
    public:
        Field()
        {
            data.value = NULL;
            data.type = MYSQL_TYPE_NULL;
            data.length = 0;
        }
        
        ~Field()
        {
            CleanUp();
        }
        
        void SetValue(char* value, enum_field_types type)
        {
            if (data.value)
                CleanUp();

            // This value stores somewhat structured data that needs function style casting
            if (value)
            {
                size_t size = strlen(value);
                data.value = new char [size+1];
                strcpy((char*)data.value, value);
                data.length = size;
            }

            data.type = type;
            data.raw = false;
        }
        
        void SetByteValue(const void* value, const size_t size, enum_field_types type, uint32 length)
        {
            if (data.value)
                CleanUp();
            
            if (value)
            {
                data.value = new char[size];
                memcpy(data.value, value, size);
                data.length = length;
            }
            
            data.type = type;
            data.raw = true;
        }
        
        template<typename T>
        T Get()
        {
            if (data.raw)
                return *reinterpret_cast<T*>(data.value);
            return boost::lexical_cast<T>(data.value);
        }
        
        static size_t SizeForType(MYSQL_FIELD* field)
        {
            switch (field->type)
            {
                case MYSQL_TYPE_NULL:
                    return 0;
                case MYSQL_TYPE_TINY:
                    return 1;
                case MYSQL_TYPE_YEAR:
                case MYSQL_TYPE_SHORT:
                    return 2;
                case MYSQL_TYPE_INT24:
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_FLOAT:
                    return 4;
                case MYSQL_TYPE_DOUBLE:
                case MYSQL_TYPE_LONGLONG:
                case MYSQL_TYPE_BIT:
                    return 8;
                case MYSQL_TYPE_TIMESTAMP:
                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_DATETIME:
                    return sizeof(MYSQL_TIME);
                case MYSQL_TYPE_TINY_BLOB:
                case MYSQL_TYPE_MEDIUM_BLOB:
                case MYSQL_TYPE_LONG_BLOB:
                case MYSQL_TYPE_BLOB:
                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VAR_STRING:
                    return field->max_length + 1;
                case MYSQL_TYPE_DECIMAL:
                case MYSQL_TYPE_NEWDECIMAL:
                    return 64;
                default:
                    return 0;
            }
        }

    private:
        struct
        {
            uint32 length;
            char* value;
            enum_field_types type;
            bool raw;
        } data;
        
        void CleanUp()
        {
            delete[] data.value;
            data.value = NULL;
        }
    };
    
    template<>
    inline const char* Field::Get()
    {
        return static_cast<const char*>(data.value);
    }
    
    template<>
    inline std::string Field::Get()
    {
        if (data.raw)
            return std::string(Get<const char*>(), data.length);
        return boost::lexical_cast<std::string>(data.value);
    }
}

#endif
