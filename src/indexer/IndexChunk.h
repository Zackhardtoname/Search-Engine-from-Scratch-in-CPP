//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_INDEXCHUNK_H
#define SCOOBLE_SEARCH_INDEXCHUNK_H

#include <cstddef>
#include <unordered_map>

#include "../stl/include/String.h"
#include "../stl/include/Vector.h"
#include "IndexerInjest.h"

constexpr static size_t INDEX_CHUNK_TARGET_SIZE = 100 * 1024 * 1024; // 100 Megabytes

using scb::String;
using scb::Vector;

// TODO: this is temporary. After we have our own hashmap implementation, ditch this
//  and use that instead.
struct StringHash {
    std::size_t operator()(const String &s) const {
        return s.hash();
    }
};

struct Document {
    String Url;
    unsigned Boundary;
};

class IndexChunk {
    size_t estimatedSize = 0;

    Vector<Document> documents;
    std::unordered_map<String, Vector<unsigned>, StringHash> postingLists;

    unsigned nextId = 0;
public:
    void ingest(IndexerIngest &injest);
};


#endif //SCOOBLE_SEARCH_INDEXCHUNK_H
