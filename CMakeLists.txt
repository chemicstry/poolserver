# Project name!
project(PoolServer)

# CMake policies
cmake_minimum_required(VERSION 2.6)

# Set macros
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

# build in Release-mode by default if not explicitly set
if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Install path
if( PREFIX )
  set(CMAKE_INSTALL_PREFIX "${PREFIX}")
endif()

# Build options
include(cmake/options.cmake)

set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(Boost_ALL_DYN_LINK           ON)

SET(Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0")

# Boost
find_package(Boost 1.49 COMPONENTS thread chrono program_options date_time system REQUIRED)
message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
message(status "** Boost Libraries: ${Boost_LIBRARIES}")

# Mysql
if( MYSQL )
  find_package(MySQL REQUIRED)
endif()

# GMP
find_package(GMP REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Print options
include(cmake/showoptions.cmake)

# Add sources
add_subdirectory(src)
