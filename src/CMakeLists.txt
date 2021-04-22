cmake_minimum_required(VERSION 3.10)
project(scooble_search)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("building as release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -DVERY_VERBOSE")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(crawler)
add_subdirectory(crawler-parser-worker)
add_subdirectory(filter)
add_subdirectory(indexer)
add_subdirectory(server)
add_subdirectory(utils)
add_subdirectory(constraint-solver)
add_subdirectory(stl)


