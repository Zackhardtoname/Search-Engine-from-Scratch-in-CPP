//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H
#include <cassert>
#include <netdb.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "io/utils.h"
#include <iostream>
#include <utility>
#include <stdexcept>

#include "../stl/include/String.h"

using scb::String;

struct NetworkException : std::runtime_error {
    NetworkException() : std::runtime_error("network error") {};

    explicit NetworkException(const char *s) : std::runtime_error(s) {};
};

String FetchHtml(const ParsedUrl *url);

#endif //CRAWLER_REQUESTER_H
