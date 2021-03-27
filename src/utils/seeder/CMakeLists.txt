cmake_minimum_required(VERSION 3.10)
project(seeder)

set(CMAKE_CXX_STANDARD 20)

add_executable(seeder Seeder.cpp ../../stl/String.cpp)
target_link_libraries(seeder rt)