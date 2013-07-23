#ifndef UTIL_H
#define UTIL_H

#include <sstream>
#include <iostream>
#include <boost/date_time.hpp>

namespace Util
{
	std::string Date(const char* format, bool utc = false)
	{
		boost::posix_time::ptime now;
		if (utc)
			now = boost::posix_time::second_clock::universal_time();
		else
			now = boost::posix_time::second_clock::local_time();

		std::stringstream ss;
		boost::posix_time::time_facet *facet = new boost::posix_time::time_facet(format);
		ss.imbue(std::locale(std::cout.getloc(), facet));
		ss << now;

		return ss.str();
	}
}

#endif