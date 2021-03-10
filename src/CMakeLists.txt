cmake_minimum_required(VERSION
        3.17)
project(scooble_search)

set(CMAKE_CXX_STANDARD
        20)

add_subdirectory(stl)

add_executable(crawler-side
        entry.cpp
        crawl-pipeline/CrawlSidePipeline.cpp
        crawl-pipeline/crawler-parser/CrawlerParserManager.cpp
        crawl-pipeline/crawler-parser/CrawlerParser.cpp
        crawl-pipeline/crawler-parser/ParsedUrl.cpp crawl-pipeline/crawler-parser/ParsedUrl.h)
target_link_libraries(crawler-side stl)
target_include_directories(crawler-side PRIVATE crawl-pipeline)