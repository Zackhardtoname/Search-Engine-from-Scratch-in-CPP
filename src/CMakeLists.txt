cmake_minimum_required(VERSION 3.17)
project(scooble_search)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenSSL REQUIRED)

add_subdirectory(stl)

add_executable(crawler-side
        crawler/Crawler.cpp
        crawler/ParsedHttpResponse.cpp
        indexer/VLQ.cpp
        crawler/Robotstxt.cpp
        stl/src/String.cpp
        crawler/ParsedUrl.cpp)
target_link_libraries(crawler-side stl ssl)

add_executable(indexer
        indexer/indexer.cpp
        indexer/IndexChunk.cpp indexer/IndexChunk.h indexer/PorterStemming.cpp indexer/PorterStemming.h)

# testing

find_package(GTest)

add_executable(testRobotstxt
        crawler/testRobotstxt.cpp
        stl/src/String.cpp
        crawler/Robotstxt.cpp)
target_link_libraries(testRobotstxt stl ssl GTest::gtest)
gtest_discover_tests(testRobotstxt)

add_executable(testVLQEncoding
        indexer/VLQ.cpp
        indexer/testVLQ.cpp)
target_link_libraries(testVLQEncoding stl ssl GTest::gtest)
gtest_discover_tests(testVLQEncoding)

add_executable(testPorterStemming
        indexer/testPorterStemming.cpp
        indexer/PorterStemming.cpp)
target_link_libraries(testPorterStemming stl GTest::gtest)
#gtest_discover_tests(testPorterStemming)
