//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H

#include "../stl/String.h"

using scb::String;

/// Fetch the given url with HTTP GET, and manages redirection.
String FetchUrl( const String &url );

#endif // CRAWLER_REQUESTER_H
