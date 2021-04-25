cmake_minimum_required(VERSION 3.10)
project(reload-urls)

set(CMAKE_CXX_STANDARD 17)

add_executable(reload-filtered-urls ReloadUrls.cpp ../../stl/String.cpp ../../stl/utils.cpp)
target_link_libraries(reload-filtered-urls rt)