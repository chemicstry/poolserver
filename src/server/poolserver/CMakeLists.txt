
# Add sources
file(GLOB_RECURSE sources_Server Server/*.cpp Server/*.h)
file(GLOB_RECURSE sources_Database Database/*.cpp Database/*.h)
file(GLOB sources_localdir *.cpp *.h)

set(sources_Poolserver
  ${sources_Server}
  ${sources_Database}
  ${sources_localdir}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/poolserver/Server
  ${CMAKE_SOURCE_DIR}/src/server/poolserver/Database
  ${Boost_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

# Create executable
add_executable(poolserver
  ${sources_Poolserver}
)

# Link libraries
target_link_libraries(poolserver
  shared
  ${Boost_LIBRARIES}
  ${MYSQL_LIBRARY}
)

# Install
if(UNIX)
  install(TARGETS poolserver DESTINATION bin)
  install(FILES poolserver.cfg.dist DESTINATION bin)
elseif(WIN32)
  install(TARGETS poolserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES poolserver.cfg.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
