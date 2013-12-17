#ifndef FIELD_MYSQL_H_
#define FIELD_MYSQL_H_

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
        }
        
        uint32_t GetUInt32()
        {
            return boost::lexical_cast<uint32_t>(data.value);
        }
        
        std::string GetString()
        {
            return boost::lexical_cast<std::string>(data.value);
        }
        
        double GetDouble()
        {
            return boost::lexical_cast<double>(data.value);
        }

    private:
        struct
        {
            uint32_t length;
            char* value;
            enum_field_types type;
        } data;
        
        void CleanUp()
        {
            delete[] data.value;
            data.value = NULL;
        }
    };
}

#endif
