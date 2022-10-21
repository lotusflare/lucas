cmake_minimum_required(VERSION 3.22)
project(lucas)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 17)
set(CC clang)
set(CXX clang++)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(vendor/cpp-driver)
include_directories(vendor/cpp-driver/include)
include_directories(include)
include_directories(src)

add_library(lucas SHARED src/library.c)
set_target_properties(lucas PROPERTIES PREFIX "")
set_target_properties(lucas PROPERTIES PUBLIC_HEADER include/library.h)
target_link_libraries(lucas cassandra luajit-5.1)

enable_testing()
add_executable(lucas_test tests/lucas_test.cpp)
target_link_libraries(lucas_test GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(lucas_test)
