cmake_minimum_required(VERSION 3.17)
project(indexer)

set(CMAKE_CXX_STANDARD 20)

add_executable(indexer
        Worker.cpp
        IndexChunk.cpp
        IndexChunk.h
        PorterStemming.cpp
        )
target_link_libraries(indexer rt)

add_executable(dummy-indexer
        DummyIndexerWorker.cpp
        )
target_link_libraries(dummy-indexer rt)

add_executable(create_test_chunk
        create_test_chunk.cpp
        IndexChunk.cpp
        VLQ.cpp
        )
target_link_libraries(create_test_chunk rt)

# testing