cmake_minimum_required(VERSION 3.17)
project(utils)

set(CMAKE_CXX_STANDARD 20)


add_executable(ReadDumpFile
        ReadDumpFile.cpp
        )

add_executable(HandleMQ
        HandleMQs.cpp
        )

add_executable(MakeBloomFile
        MakeBloomFile.cpp
        )

# testing