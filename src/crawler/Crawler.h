//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H

#include "../stl/String.h"
#include <curl/curl.h>

using scb::String;

CURL *GetCurlHandle( String *content );

#endif // CRAWLER_REQUESTER_H
