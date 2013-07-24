#ifndef DATABASE_CALLBACK_H_
#define DATABASE_CALLBACK_H_

#include "QueryResult.h"

#include <boost/function.hpp>

typedef boost::function<void(ResultSet*)> DatabaseCallback;

#endif