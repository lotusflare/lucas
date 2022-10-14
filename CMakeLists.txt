cmake_minimum_required(VERSION 3.22)
project(cmake_cassandra_lua)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(vendor/cpp-driver)

set(CASS_BUILD_SHARED OFF)
set(CASS_BUILD_STATIC ON)
set(CASS_USE_STATIC_LIBS OFF)

include_directories(vendor/cpp-driver/include)

add_library(cmake_cassandra_lua SHARED library.cpp)
