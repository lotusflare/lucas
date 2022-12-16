cmake_minimum_required(VERSION 3.16)
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
set(CC clang)
set(CXX clang++)

find_package(Git)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --match "v[0-9].[0-9].[0-9]"
    OUTPUT_VARIABLE GIT_DESCRIBE
    RESULT_VARIABLE GIT_DESCRIBE_RETURN_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX REPLACE "^v([0-9]+).[0-9]+.[0-9]+" "\\1" VERSION_MAJOR "${GIT_DESCRIBE}")
message(STATUS "lucas version: ${GIT_DESCRIBE}")
string(REGEX REPLACE "^v[0-9]+.([0-9]+).[0-9]+" "\\1" VERSION_MINOR "${GIT_DESCRIBE}")
string(REGEX REPLACE "^v[0-9]+.[0-9]+.([0-9]+)" "\\1" VERSION_PATCH "${GIT_DESCRIBE}")

add_subdirectory(vendor/cpp-driver)
include_directories(vendor/cpp-driver/include)
include_directories(include)

project(lucas)
add_library(lucas SHARED src/lucas.c)
set_target_properties(lucas PROPERTIES VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set_target_properties(lucas PROPERTIES SOVERSION "${VERSION_MAJOR}")
set_target_properties(lucas PROPERTIES PREFIX "")
set_target_properties(lucas PROPERTIES PUBLIC_HEADER include/lucas.h)
target_link_libraries(lucas cassandra luajit-5.1)
