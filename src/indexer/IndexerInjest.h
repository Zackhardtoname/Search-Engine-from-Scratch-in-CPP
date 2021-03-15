//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_INDEXERINJEST_H
#define SCOOBLE_SEARCH_INDEXERINJEST_H

#include "../stl/include/String.h"
#include "../stl/include/Vector.h"

using scb::String;
using scb::Vector;

struct IndexerIngest {
    Vector<String> words;
    Vector<String> titleWords;
    String url;
};

#endif //SCOOBLE_SEARCH_INDEXERINJEST_H
