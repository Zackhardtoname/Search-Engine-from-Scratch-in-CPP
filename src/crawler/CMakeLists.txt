cmake_minimum_required(VERSION 3.10)
project(crawler)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest)

#add_executable(testParsedUrl
#        testParsedUrl.cpp
#        ParsedUrl.cpp
#        )
#target_link_libraries(testParsedUrl GTest::gtest)
#gtest_discover_tests(testParsedUrl)

add_executable(crawlerCmdline
        ../parser/ParsedHtml.cpp
        ../parser/HtmlLexer.cpp
        ParsedHttpResponse.cpp
        ParsedUrl.cpp
        Crawler.cpp
        ../stl/String.cpp
        ../stl/utils.cpp
        CrawlerCmdline.cpp
        ../indexer/PorterStemming.cpp
        )
target_link_libraries(crawlerCmdline ssl rt crypto)
