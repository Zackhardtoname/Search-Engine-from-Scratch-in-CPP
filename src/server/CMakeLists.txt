cmake_minimum_required(VERSION 3.10)
project(server)

add_executable(server
        ./LinuxTinyServer.cpp
        )

add_executable(client client.c)

target_link_libraries( server pthread )
