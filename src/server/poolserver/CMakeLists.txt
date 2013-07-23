
# Add sources
file(GLOB_RECURSE sources_Server Server/*.cpp Server/*.h)
file(GLOB sources_localdir *.cpp *.h)

set(sources_Poolserver
  ${sources_Server}
  ${sources_localdir}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/poolserver/Server
  ${Boost_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

# Create executable
add_executable(poolserver
  ${sources_Poolserver}
)
message(status "SHared files: ${sources_Shared}")

# Link libraries
target_link_libraries(poolserver
  shared
  ${MYSQL_LIBRARY}
  ${Boost_LIBRARIES}
  /usr/local/lib/libboost_thread.so.1.53.0
  /usr/local/lib/libboost_program_options.so
)

# Install
if(UNIX)
  install(TARGETS poolserver DESTINATION bin)
elseif(WIN32)
  install(TARGETS poolserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
