project(lucas)
cmake_minimum_required(VERSION 3.16)
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
set(CC clang)
set(CXX clang++)

find_package(Git)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v[0-9].[0-9].[0-9]"
    OUTPUT_VARIABLE GIT_DESCRIBE
    RESULT_VARIABLE GIT_DESCRIBE_RETURN_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "^v([0-9]+).([0-9]+).([0-9]+)((-[0-9]+)(-g[a-f0-9]+))?(-dirty)?$" _ "${GIT_DESCRIBE}")
set(VERSION_MAJOR "${CMAKE_MATCH_1}")
set(VERSION_MINOR "${CMAKE_MATCH_2}")
set(VERSION_PATCH "${CMAKE_MATCH_3}")
set(VERSION_SUFFIX "${CMAKE_MATCH_6}${CMAKE_MATCH_7}")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX}")
message(STATUS "lucas version: ${VERSION}")
add_compile_definitions(VERSION="${VERSION}")

include_directories(include)

add_library("${PROJECT_NAME}" SHARED src/lucas.c)
set_target_properties("${PROJECT_NAME}" PROPERTIES VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX}")
set_target_properties("${PROJECT_NAME}" PROPERTIES SOVERSION "${VERSION_MAJOR}")
set_target_properties("${PROJECT_NAME}" PROPERTIES PREFIX "")
set_target_properties("${PROJECT_NAME}" PROPERTIES PUBLIC_HEADER include/lucas.h)
target_link_libraries("${PROJECT_NAME}" cassandra luajit-5.1)
