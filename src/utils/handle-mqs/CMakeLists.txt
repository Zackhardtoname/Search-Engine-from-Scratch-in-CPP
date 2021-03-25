cmake_minimum_required(VERSION 3.17)
project(handle-mqs)

set(CMAKE_CXX_STANDARD 20)

add_executable(handle-mq HandleMQs.cpp)
target_link_libraries(handle-mq rt)