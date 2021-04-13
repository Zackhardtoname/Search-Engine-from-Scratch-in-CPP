#ifndef QUERY_LISTENER_H
#define QUERY_LISTENER_H

#include "../stl/String.h"

struct DocumentMatch {
    scb::String url;
    float rankScore;
};

#endif //QUERY_LISTENER_H