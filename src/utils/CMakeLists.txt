cmake_minimum_required(VERSION 3.17)
project(utils)

set(CMAKE_CXX_STANDARD 20)

add_executable(CheckMQs
        CheckMQs.cpp
        )
target_link_libraries(CheckMQs rt)

add_executable(DrainMQs
        DrainMQs.cpp
        )
target_link_libraries(DrainMQs rt)

add_executable(ReadDumpFile
        ReadDumpFile.cpp
        )

add_executable(ReadChunk
        ReadChunk.cpp
)

# testing