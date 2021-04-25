cmake_minimum_required(VERSION 3.10)
project(constraint-solver)

set(CMAKE_CXX_STANDARD 17)

add_executable(constraint-solver
        QueryListener.cpp
        ../stl/String.cpp
        ISR/AND.h ISR/Word.h QueryableChunk.cpp QueryableChunk.h ISR/Null.h)
