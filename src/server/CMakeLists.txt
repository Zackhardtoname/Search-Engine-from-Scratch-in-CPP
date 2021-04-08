cmake_minimum_required(VERSION 3.10)
project(server)

add_executable(hwserver
        ./LinuxTinyServer.cpp
        )

add_executable(client client.cpp)

target_link_libraries( hwserver pthread )
