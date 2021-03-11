cmake_minimum_required(VERSION 3.17)
project(scooble_search)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenSSL REQUIRED)

add_subdirectory(stl)

add_executable(crawler-side
        crawler/Crawler.cpp
        crawler/ParsedHttpResponse.cpp
        crawler/Robotstxt.cpp
        stl/src/String.cpp
        crawler/ParsedUrl.cpp)
target_link_libraries(crawler-side stl ssl)
