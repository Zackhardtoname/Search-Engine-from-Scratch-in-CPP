cmake_minimum_required(VERSION 3.10)
project(server)

add_executable(server
        Server.cpp
        ../stl/utils.cpp
        ../indexer/PorterStemming.cpp
        ../indexer/VLQ.cpp
        )

add_executable(client Client.cpp)

target_link_libraries( server pthread )
