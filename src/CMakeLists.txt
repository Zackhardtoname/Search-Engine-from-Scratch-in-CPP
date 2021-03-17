cmake_minimum_required(VERSION 3.17)
project(scooble_search)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenSSL REQUIRED)

add_subdirectory(stl)

add_executable(crawlerCmdline
        parser/ParsedHtml.cpp
        parser/HtmlLexer.cpp
        crawler/ParsedHttpResponse.cpp
        crawler/ParsedUrl.cpp
        crawler/Crawler.cpp
        stl/src/String.cpp
        stl/src/io/utils.cpp
        crawler/CrawlerCmdline.cpp
        indexer/PorterStemming.cpp
        )
target_link_libraries(crawlerCmdline ssl rt crypto)

add_executable(crawler-parser-worker
        parser/ParsedHtml.cpp
        parser/HtmlLexer.cpp
        crawler/ParsedHttpResponse.cpp
        crawler/ParsedUrl.cpp
        crawler/Crawler.cpp
        stl/src/String.cpp
        stl/src/io/utils.cpp
        #        crawler-parser-worker/WorkerCmdline.cpp
        crawler-parser-worker/CrawlerParserWorker.cpp
        indexer/PorterStemming.cpp
        )
target_link_libraries(crawler-parser-worker ssl rt crypto)

add_executable(dummy-filter
        filter/DummyFilterWorker.cpp
        stl/src/String.cpp
        )
target_link_libraries(dummy-filter rt)

add_executable(indexer
        indexer/Worker.cpp
        indexer/IndexChunk.cpp
        indexer/IndexChunk.h
        indexer/PorterStemming.cpp
        )

add_executable(CheckMQs
        utils/CheckMQs.cpp
        )
target_link_libraries(CheckMQs rt)

add_executable(DrainMQs
        utils/DrainMQs.cpp
        )
target_link_libraries(DrainMQs rt)

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
gtest_discover_tests(testPorterStemming)
