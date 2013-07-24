
file(GLOB_RECURSE sources_MySQL MySQL/*.cpp MySQL/*.h)
file(GLOB sources_localdir *.cpp *.h)

set(sources_Database
  ${sources_localdir}
  PARENT_SCOPE
)

if(MYSQL)
  set(sources_Database
    ${sources_Database}
    ${sources_MySQL}
    ${MYSQL_INCLUDE_DIR}
    PARENT_SCOPE
  )
endif()
