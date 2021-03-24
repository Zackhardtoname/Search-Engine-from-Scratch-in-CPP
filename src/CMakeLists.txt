cmake_minimum_required(VERSION 3.17)
project(scooble_search)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenSSL REQUIRED)

add_subdirectory(crawler)
add_subdirectory(indexer)
add_subdirectory(filter)
add_subdirectory(utils)
add_subdirectory(crawler-controller)
add_subdirectory(crawler-parser-worker)

add_executable(crawlerCmdline
        parser/ParsedHtml.cpp
        parser/HtmlLexer.cpp
        crawler/ParsedHttpResponse.cpp
        crawler/ParsedUrl.cpp
        crawler/Crawler.cpp
        stl/String.cpp
        stl/utils.cpp
        crawler/CrawlerCmdline.cpp
        indexer/PorterStemming.cpp
        )
target_link_libraries(crawlerCmdline ssl rt crypto)

add_executable(test
        test_file.cpp
)


# testing

find_package(GTest)

add_executable(testRobotstxt
        filter/testRobotstxt.cpp
        stl/String.cpp
        filter/Robotstxt.cpp)
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
gtest_discover_tests(testPorterStemming)
