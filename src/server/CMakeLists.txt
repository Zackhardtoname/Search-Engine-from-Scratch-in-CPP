cmake_minimum_required(VERSION 3.10)
project(server)

add_executable(server
        server.cpp
        mime.h
        )

add_executable(client client.cpp)

target_link_libraries( server pthread )
