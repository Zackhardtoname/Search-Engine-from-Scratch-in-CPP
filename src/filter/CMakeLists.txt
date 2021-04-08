cmake_minimum_required(VERSION 3.10)
project(filter)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)

add_executable(constant-filter
        ConstantFilterWorker.cpp
        ../stl/String.cpp
        )
target_link_libraries(constant-filter rt)

add_executable(dummy-filter
        DummyFilterWorker.cpp
        ../stl/String.cpp
        )
target_link_libraries(dummy-filter rt)

add_executable(filter-worker
        FilterWorker.cpp
        Robotstxt.cpp
        ../crawler/Crawler.cpp
        ../crawler/ParsedHttpResponse.cpp
        ../crawler/ParsedUrl.cpp
        ../stl/String.cpp
        ../stl/utils.cpp
        ThreadNewLinksMqueue.cpp
        ThreadOffloadUnfilteredUrls.cpp
        ThreadRemoteWorkers.cpp
        ThreadRemoteUrls.cpp
        ThreadFilteredUrls.cpp)
target_link_libraries(filter-worker rt Threads::Threads ssl crypto)

# testing

#find_package(GTest)
#
#add_executable(testRobotstxt
#        testRobotstxt.cpp
#        ../stl/String.cpp
#        Robotstxt.cpp)
#target_link_libraries(testRobotstxt ssl GTest::gtest)
#gtest_discover_tests(testRobotstxt)
