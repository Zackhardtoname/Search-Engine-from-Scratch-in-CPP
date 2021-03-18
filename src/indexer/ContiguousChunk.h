//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H
#define SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H

#include <cstddef>
#include <unordered_map>

#include "../stl/String.h"
#include "../stl/Vector.h"
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

// class SyncEntry {
//     unsigned seekOffsetDelta;
//     unsigned postLocationDelta;
// };

// class PostArray {
//     unsigned posts[];
// };

// class SyncTable {
//     SyncEntry entries[];
// };

// class PostingList {
//     SyncTable syncTable;
//     PostArray posts;
// };

struct ChunkHeader {
    uint32_t docCount;
    uint32_t urlListOffset;
    uint32_t dictionaryOffset;
    uint32_t postingListsOffset;
};

struct ContiguousChunk {
public:
    uint32_t docCount;
    uint32_t urlListOffset;
    uint32_t dictionaryOffset;
    uint32_t postingListsOffset;
    void writeToDisk();
};


#endif //SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H
