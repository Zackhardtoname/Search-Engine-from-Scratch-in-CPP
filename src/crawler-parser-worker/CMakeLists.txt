cmake_minimum_required(VERSION 3.10)
project(crawler-parser-worker)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)

add_executable(crawler-group ../parser/ParsedHtml.cpp
        ../crawler/ParsedHttpResponse.cpp
        ../crawler/ParsedUrl.cpp
        ../crawler/Crawler.cpp ../stl/String.cpp
        ../stl/utils.cpp
        ../indexer/VLQ.cpp CrawlerParserWorker.cpp CrawlerGroup.cpp
        ../indexer/PorterStemming.cpp)

target_link_libraries(crawler-group rt Threads::Threads)

#testing