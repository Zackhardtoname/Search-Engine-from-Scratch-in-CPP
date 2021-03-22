cmake_minimum_required(VERSION 3.17)
project(filter)

set(CMAKE_CXX_STANDARD 20)

add_executable(dummy-filter
        DummyFilterWorker.cpp
        ../stl/String.cpp
        )
target_link_libraries(dummy-filter rt)

add_executable(filter-worker
        FilterWorker.cpp
        ../stl/String.cpp
        )
target_link_libraries(filter-worker rt)

# testing