//
// Created by youh on 3/2/21.
//

#ifndef SCOOBLE_SEARCH_CRAWLERPARSER_H
#define SCOOBLE_SEARCH_CRAWLERPARSER_H

#include <mqueue.h>

#include "String.h"
#include "ParsedUrl.h"

class FetchResult {
public:
    // The final url the crawler actually uses
    ParsedUrl url;
    int StatusCode;
    String Lang;
    String RespHeader;
    String RespBody;

    FetchResult(const ParsedUrl &url,
                int statusCode, String lang,
                String respHeader, String respBody);
};

std::string getHtmlResponseWithSockets(const ParsedUrl *url);

class CrawlerParser {
    mqd_t urlMQ;

    FetchResult fetch(const String &url);
public:
    CrawlerParser(mqd_t urlMQ);

    void RunForever();
};

#endif //SCOOBLE_SEARCH_CRAWLERPARSER_H
