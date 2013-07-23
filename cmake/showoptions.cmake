# output generic information about the core and buildtype chosen
message("")
# message("* PoolServer revision   : ${rev_hash} ${rev_date} (${rev_branch} branch)")
if( UNIX )
  message("* Buildtype              : ${CMAKE_BUILD_TYPE}")
endif()
message("")

# output information about installation-directories and locations

message("* Install core to        : ${CMAKE_INSTALL_PREFIX}")
message("")

# Show infomation about the options selected during configuration

if(POOLSERVER)
  message("* Build Pool Server      : Yes (default)")
else()
  message("* Build Pool Server      : No")
endif()

if(STRATUM)
  message("* Build with Stratum     : Yes (default)")
  add_definitions(-DWITH_STRATUM)
else()
  message("* Build with Stratum     : No")
endif()

if(STATSSERVER)
  message("* Build Stats Server     : Yes (default)")
else()
  message("* Build Stats Server     : No")
endif()

if( MYSQL )
  message("* Use MySQL database     : Yes (default)")
  add_definitions(-DWITH_MYSQL)
else()
  message("* Use MySQL database     : No")
endif()

message("")
