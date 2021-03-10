//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H

#include "../stl/include/String.h"

using agr::String;

class FetchResult {
public:
    // The final url the crawler actually uses
    ParsedUrl url;
    int statusCode;
    std::string language;
    std::string responseHeader;
    std::string responseBody;

    FetchResult(const ParsedUrl &url,
                int statusCode, std::string language,
                std::string responseHeader, std::string responseBody)
            : url(url), statusCode(statusCode),
              language(std::move(language)), responseHeader(std::move(responseHeader)),
              responseBody(std::move(responseBody)) {}
};

std::string FetchHtml(const ParsedUrl *url);

FetchResult fetch(const String &urlString);

[[noreturn]] void run();
#endif //CRAWLER_REQUESTER_H
