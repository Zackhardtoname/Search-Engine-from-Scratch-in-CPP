cmake_minimum_required(VERSION 3.17)
project(crawler-parser-worker)

set(CMAKE_CXX_STANDARD 20)
find_package(Threads REQUIRED)

add_executable(crawler-parser-worker
        ../parser/ParsedHtml.cpp
        ../parser/HtmlLexer.cpp
        ../crawler/ParsedHttpResponse.cpp
        ../crawler/ParsedUrl.cpp
        ../crawler/Crawler.cpp
        ../stl/String.cpp
        ../stl/utils.cpp
        ../indexer/VLQ.cpp
        #        crawler-parser-worker/WorkerCmdline.cpp
        CrawlerParserWorker.cpp
        ../indexer/PorterStemming.cpp
        )
target_link_libraries(crawler-parser-worker ssl rt crypto)
# testing