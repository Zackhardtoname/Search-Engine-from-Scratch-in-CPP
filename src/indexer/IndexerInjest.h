//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_INDEXERINJEST_H
#define SCOOBLE_SEARCH_INDEXERINJEST_H

#include "../stl/String.h"
#include "../stl/Vector.h"

using scb::String;
using scb::Vector;

struct IndexerIngest {
    Vector<String> Words;
    Vector<String> TitleWords;
    String Url;

    String Serialize();

    explicit IndexerIngest(const String &s);
};

#endif //SCOOBLE_SEARCH_INDEXERINJEST_H
