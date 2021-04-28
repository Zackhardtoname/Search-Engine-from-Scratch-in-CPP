cmake_minimum_required(VERSION 3.10)
project(server)

find_package(CURL REQUIRED)

add_executable(server
        Server.cpp
        ../indexer/PorterStemming.cpp
        ../stl/utils.cpp
        ../indexer/VLQ.cpp
        ../crawler/Crawler.cpp
        ../crawler/ParsedUrl.cpp
        ../parser/ParsedHtml.cpp
        OnTheFlyLoader.cpp OnTheFlyLoader.h JSON.h)

add_executable(client Client.cpp)

target_link_libraries(server pthread ${CURL_LIBRARIES})
