cmake_minimum_required(VERSION 3.17)
project(read-chunk)

set(CMAKE_CXX_STANDARD 20)

add_executable(read-chunk ReadChunk.cpp ../../indexer/VLQ.cpp)