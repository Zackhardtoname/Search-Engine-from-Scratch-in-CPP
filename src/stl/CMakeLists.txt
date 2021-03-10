cmake_minimum_required(VERSION 3.17)
project(stl)

set(CMAKE_CXX_STANDARD 20)

add_library(stl SHARED
        src/wire.cpp
        src/io/utils.cpp)
target_include_directories(stl PUBLIC ./include)
