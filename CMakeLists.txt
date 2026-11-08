cmake_minimum_required(VERSION 3.15)
project(concurrency-in-action LANGUAGES CXX)
add_subdirectory(external/googletest)
set(CMAKE_CXX_STANDARD 23)
add_executable(concurrency-in-action 
    2-managing-threads.cpp
)
target_link_libraries(concurrency-in-action gtest_main)
include(GoogleTest)
gtest_discover_tests(concurrency-in-action)
enable_testing()