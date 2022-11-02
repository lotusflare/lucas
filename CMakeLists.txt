cmake_minimum_required(VERSION 3.16)
project(lucas)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
set(CC clang)
set(CXX clang++)

add_subdirectory(vendor/cpp-driver)
include_directories(vendor/cpp-driver/include)
include_directories(include)

add_library(lucas SHARED src/lucas.c)
set_target_properties(lucas PROPERTIES PREFIX "")
set_target_properties(lucas PROPERTIES PUBLIC_HEADER include/library.h)
target_link_libraries(lucas cassandra luajit-5.1)
