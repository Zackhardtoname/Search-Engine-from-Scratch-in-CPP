cmake_minimum_required(VERSION 3.17)
project(crawler-controller)

set(CMAKE_CXX_STANDARD 20)

add_executable(crawler-controller
        CrawlerController.cpp
        )

# testing