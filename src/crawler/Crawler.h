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
#include "../stl/utils.h"
#include <iostream>
#include <utility>
#include <stdexcept>

#include "../stl/String.h"
#include "ParsedHttpResponse.h"
#include "ParsedUrl.h"

using scb::String;

constexpr int MAX_REDIRECTION = 5;
constexpr int MAX_RESPONSE_SIZE = 102400;

struct NetworkException : std::runtime_error {
    NetworkException() : std::runtime_error("network error") {};

    explicit NetworkException(const char *s) : std::runtime_error(s) {};
};

struct HttpRequestError : std::runtime_error {
    HttpRequestError() : std::runtime_error("HTTP request error") {};

    explicit HttpRequestError(const char *s) : std::runtime_error(s) {};
};

struct TooManyRedirectionError : HttpRequestError {
    TooManyRedirectionError() : HttpRequestError("too many redirection") {}
    explicit TooManyRedirectionError(const char *s) : HttpRequestError(s) {}
};

/// Send an HTTP GET request and returns the response.
String HttpGet(const ParsedUrl *url);

/// Fetch the given url with HTTP GET, and manages redirection.
ParsedHttpResponse FetchUrl(String url);

#endif //CRAWLER_REQUESTER_H
