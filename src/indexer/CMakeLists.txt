cmake_minimum_required(VERSION 3.10)
project(indexer)

set(CMAKE_CXX_STANDARD 20)

#add_executable(indexer-worker
#        Worker.cpp
#        IndexChunk.cpp
#        IndexChunk.h
#        PorterStemming.cpp
#        )
#target_link_libraries(indexer-worker rt)

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
find_package(GTest)

add_executable(testVLQEncoding VLQ.cpp testVLQ.cpp)
target_link_libraries(testVLQEncoding ssl GTest::gtest)
gtest_discover_tests(testVLQEncoding)

add_executable(testPorterStemming testPorterStemming.cpp PorterStemming.cpp)
target_link_libraries(testPorterStemming GTest::gtest)
gtest_discover_tests(testPorterStemming)
