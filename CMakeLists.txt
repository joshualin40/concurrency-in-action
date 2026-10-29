cmake_minimum_required(VERSION 3.15)
project(preply_practice LANGUAGES CXX)
add_subdirectory(external/googletest)
set(CMAKE_CXX_STANDARD 23)
add_executable(preply_practice 
    main.cpp
    smartpointerpractice.cpp
)
target_link_libraries(preply_practice gtest_main)
include(GoogleTest)
gtest_discover_tests(preply_practice)
enable_testing()