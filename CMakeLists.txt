cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0048 NEW)

set(LUCAS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LUCAS_SRC_DIR "${LUCAS_ROOT_DIR}/src")
set(LUCAS_INCLUDE_DIR "${LUCAS_ROOT_DIR}/include")
set(version_header_file "${LUCAS_INCLUDE_DIR}/lucas.h")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
set(CC clang)
set(CXX clang++)

set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

#------------------------
# Project Version
#------------------------
foreach(level MAJOR MINOR PATCH)
    file(STRINGS "${version_header_file}"
                 _define_${level}
                 REGEX "#define LUCAS_VERSION_${level}[ \t]+([0-9]+|\"([^\"]+)\")")
    string(REGEX MATCH "([0-9]+|\"([^\"]+)\")" _version_${level} "${_define_${level}}")
endforeach()

set(lucas_VERSION_STRING "${_version_MAJOR}.${_version_MINOR}.${_version_PATCH}")
message(STATUS "lucas library version: ${lucas_VERSION_STRING}")
project(lucas VERSION ${lucas_VERSION_STRING} DESCRIPTION "LUA C integration layer for DataStax Cassandra CPP Driver access")

#------------------------
# Datastax cassandra CPP driver shared libraries
#------------------------
add_subdirectory(vendor/cpp-driver)
include_directories(vendor/cpp-driver/include)
include_directories(include)

#------------------------
# LuCas LUA integration layer shared libraries
#------------------------
add_library(lucas SHARED "${LUCAS_SRC_DIR}/lucas.c")
set_target_properties(lucas PROPERTIES VERSION ${lucas_VERSION_STRING})
set_target_properties(lucas PROPERTIES PREFIX "")
set_target_properties(lucas PROPERTIES PUBLIC_HEADER "${LUCAS_INCLUDE_DIR}/lucas.h")
target_link_libraries(lucas cassandra luajit-5.1)
