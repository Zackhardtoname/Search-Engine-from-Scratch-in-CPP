//
// Created by youh on 3/2/21.
//

#ifndef CRAWLER_REQUESTER_H
#define CRAWLER_REQUESTER_H

#include "../stl/include/String.h"

using scb::String;

String FetchHtml(const ParsedUrl *url);

#endif //CRAWLER_REQUESTER_H
