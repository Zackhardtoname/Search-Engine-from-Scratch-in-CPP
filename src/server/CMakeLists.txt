cmake_minimum_required(VERSION 3.10)
project(server)

add_executable(server
	Server.cpp
	Mime.h
        )

add_executable(client Client.cpp)

target_link_libraries( server pthread )
