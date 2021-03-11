//
// Created by youh on 3/9/21.
//

#ifndef SCOOBLE_SEARCH_PARSEDHTTPRESPONSE_H
#define SCOOBLE_SEARCH_PARSEDHTTPRESPONSE_H

#include <stdexcept>

#include "../stl/include/String.h"

using scb::String;

static constexpr const char *CONTENT_LANGUAGE_HEADER = "Content-Language:";
static constexpr const char *LOCATION_HEADER = "Location:";
static constexpr const char *TRANSFER_ENCODING_HEADER = "Transfer-Encoding:";
static constexpr const char *CONTENT_TYPE_HEADER = "Content-Type:";

struct HttpParserError : std::invalid_argument {
    HttpParserError() : std::invalid_argument("invalid HTTP response") {}

    explicit HttpParserError(const char *s) : std::invalid_argument(s) {}
};

class ParsedHttpResponse {
    static String parseHeaderValue(const String &headerLine, const String &key);

public:
    String Code, Language, Location, Body, ContentType;

    ParsedHttpResponse(const String &resp);
};


#endif //SCOOBLE_SEARCH_PARSEDHTTPRESPONSE_H
