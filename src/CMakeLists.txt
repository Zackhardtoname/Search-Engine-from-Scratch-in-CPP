cmake_minimum_required(VERSION 3.10)
project(scooble_search)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(crawler)
add_subdirectory(crawler-parser-worker)
add_subdirectory(filter)
add_subdirectory(indexer)
add_subdirectory(utils)


