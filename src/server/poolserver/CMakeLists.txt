
# Add sources
file(GLOB_RECURSE sources_Server Server/*.cpp Server/*.h)
file(GLOB_RECURSE sources_Database Database/*.cpp Database/*.h)
file(GLOB_RECURSE sources_Stratum Stratum/*.cpp Stratum/*.h)
file(GLOB_RECURSE sources_DataMgr DataMgr/*.cpp DataMgr/*.h)
file(GLOB_RECURSE sources_NetworkMgr NetworkMgr/*.cpp NetworkMgr/*.h)
file(GLOB sources_localdir *.cpp *.h)

set(sources_Poolserver
  ${sources_Server}
  ${sources_Database}
  ${sources_Stratum}
  ${sources_DataMgr}
  ${sources_NetworkMgr}
  ${sources_localdir}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Server
  ${CMAKE_CURRENT_SOURCE_DIR}/Database
  ${CMAKE_CURRENT_SOURCE_DIR}/DataMgr
  ${CMAKE_CURRENT_SOURCE_DIR}/NetworkMgr
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/MySQL
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/JSON
  ${CMAKE_SOURCE_DIR}/src/server/shared/JSONRPC
  ${CMAKE_SOURCE_DIR}/src/server/shared/Bitcoin
  ${Boost_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${GMP_INCLUDE_DIR}
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
  ${GMP_LIBRARIES}
  ${GMPXX_LIBRARIES}
  ${OPENSSL_LIBRARIES}
)

# Install
if(UNIX)
  install(TARGETS poolserver DESTINATION bin)
  install(FILES poolserver.cfg.dist DESTINATION etc)
elseif(WIN32)
  install(TARGETS poolserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES poolserver.cfg.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()
