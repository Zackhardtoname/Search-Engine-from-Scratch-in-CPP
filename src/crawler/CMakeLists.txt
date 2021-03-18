cmake_minimum_required(VERSION 3.17)
project(crawler)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest)

add_executable(testParsedUrl
        testParsedUrl.cpp
        ParsedUrl.cpp
        )
target_link_libraries(testParsedUrl GTest::gtest)
gtest_discover_tests(testParsedUrl)