//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H
#include "../stl/utils.h"
#include <cassert>
#include <iostream>
#include <netdb.h>
#include <openssl/ssl.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "../stl/String.h"
#include "ParsedHttpResponse.h"
#include "ParsedUrl.h"

using scb::String;

/// Fetch the given url with HTTP GET, and manages redirection.
String FetchUrl( const String &url );

#endif // CRAWLER_REQUESTER_H
