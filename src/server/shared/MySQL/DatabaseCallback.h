#ifndef DATABASE_CALLBACK_H_
#define DATABASE_CALLBACK_H_

#include "QueryResult.h"

#include <boost/function.hpp>

namespace MySQL
{
    typedef boost::function<void(QueryResult)> DatabaseCallback;
}

#endif
