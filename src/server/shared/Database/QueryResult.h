#ifndef RESULTSET_H_
#define RESULTSET_H_

#include "Field.h"
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

class ResultSet
{
public:
    // Metadata
    virtual uint64_t GetRowCount() = 0;
    virtual uint32_t GetFieldCount() = 0;
    
    virtual bool NextRow() = 0;
    virtual Field* Fetch() = 0;
};

typedef boost::shared_ptr<ResultSet> QueryResult;

#endif