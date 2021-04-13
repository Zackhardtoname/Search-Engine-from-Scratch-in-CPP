cmake_minimum_required(VERSION 3.10)
project(indexer)

set(CMAKE_CXX_STANDARD 17)

# Necessary to link with pthreads on Michael's computer
find_package(Threads REQUIRED)

add_executable(indexer-worker
       IndexerWorker.cpp
       IndexChunk.cpp
       IndexChunk.h
       PorterStemming.cpp
       IndexerIngest.h
       VLQ.cpp
       )
target_link_libraries(indexer-worker rt Threads::Threads)

add_executable(dummy-indexer
        DummyIndexerWorker.cpp
        )
target_link_libraries(dummy-indexer rt)

add_executable(dump-indexer
        IndexChunk.cpp
        IndexChunk.h
        PorterStemming.cpp
        IndexerIngest.h
        VLQ.cpp
        DumpIndexer.cpp
        ../stl/String.cpp
        ../stl/utils.cpp
        )
target_link_libraries(indexer-worker rt Threads::Threads)

# add_executable(create_test_chunk
#         create_test_chunk.cpp
#         IndexChunk.cpp
#         VLQ.cpp
#         )
# target_link_libraries(create_test_chunk rt)

# testing
#find_package(GTest)
#
#add_executable(testVLQEncoding VLQ.cpp testVLQ.cpp)
#target_link_libraries(testVLQEncoding ssl GTest::gtest)
#gtest_discover_tests(testVLQEncoding)
#
#add_executable(testPorterStemming testPorterStemming.cpp PorterStemming.cpp)
#target_link_libraries(testPorterStemming GTest::gtest)
#gtest_discover_tests(testPorterStemming)
