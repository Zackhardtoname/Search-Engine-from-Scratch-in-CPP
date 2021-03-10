//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H

#include <stdexcept>

#include "../stl/include/String.h"

using scb::String;

struct NetworkException : std::runtime_error {
    NetworkException() : std::runtime_error("network error");

    NetworkException(const char *s) : std::runtime_error(s);
};

String FetchHtml(const ParsedUrl *url);

#endif //CRAWLER_REQUESTER_H
