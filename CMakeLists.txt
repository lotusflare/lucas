cmake_minimum_required(VERSION 3.22)
project(cmake_cassandra_lua)
set(CMAKE_CXX_STANDARD 14)
set(CASS_BUILD_STATIC OFF)
set(CASS_BUILD_STATIC ON)
set(CASS_USE_STATIC_LIBS ON)

# CMake dependencies
add_subdirectory(vendor/libuv)
add_subdirectory(vendor/cpp-driver)

add_library(cmake_cassandra_lua SHARED library.cpp)
