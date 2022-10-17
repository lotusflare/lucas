cmake_minimum_required(VERSION 3.22)
project(luacassandra)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(vendor/cpp-driver)
include_directories(vendor/cpp-driver/include)

add_library(luacassandra SHARED library.c)
set_target_properties(luacassandra PROPERTIES PREFIX "")
set_target_properties(luacassandra PROPERTIES PUBLIC_HEADER include/library.h)
target_link_libraries(luacassandra cassandra)
