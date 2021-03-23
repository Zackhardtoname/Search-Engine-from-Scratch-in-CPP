cmake_minimum_required(VERSION 3.17)
project(filter)

set(CMAKE_CXX_STANDARD 20)
find_package(Threads REQUIRED)

add_executable(dummy-filter
        DummyFilterWorker.cpp
        ../stl/String.cpp
        )
target_link_libraries(dummy-filter rt)

add_executable(filter-worker
        FilterWorker.cpp
        Robotstxt.cpp
        ../crawler/Crawler.cpp
        ../crawler/ParsedHttpResponse.cpp
        ../crawler/ParsedUrl.cpp
        ../stl/String.cpp
        ../stl/utils.cpp
        )
target_link_libraries(filter-worker rt Threads::Threads ssl crypto)

# testing