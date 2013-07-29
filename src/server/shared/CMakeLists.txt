
add_subdirectory(Database)

file(GLOB_RECURSE sources_Configuration Configuration/*.cpp Configuration/*.h)
file(GLOB_RECURSE sources_Logging Logging/*.cpp Logging/*.h)
file(GLOB_RECURSE sources_Stratum Stratum/*.cpp Stratum/*.h)
file(GLOB_RECURSE sources_JSON JSON/*.cpp JSON/*.h)
file(GLOB_RECURSE sources_JSONRPC JSONRPC/*.cpp JSONRPC/*.h)

file(GLOB sources_localdir *.cpp *.h)

set(sources_Shared
  ${sources_Configuration}
  ${sources_Database}
  ${sources_Logging}
  ${sources_Stratum}
  ${sources_JSON}
  ${sources_JSONRPC}
  ${sources_localdir}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Configuration
  ${CMAKE_CURRENT_SOURCE_DIR}/Database
  ${CMAKE_CURRENT_SOURCE_DIR}/Logging
  ${CMAKE_CURRENT_SOURCE_DIR}/JSON
  ${CMAKE_CURRENT_SOURCE_DIR}/JSONRPC
  ${Boost_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

add_library(shared STATIC
  ${sources_Shared}
)
