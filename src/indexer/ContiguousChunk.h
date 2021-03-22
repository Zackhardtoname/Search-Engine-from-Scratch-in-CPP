//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H
#define SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H

#include <cstddef>
#include <unordered_map>

#include "../defs.h"
#include "../stl/String.h"
#include "../stl/Vector.h"
#include "IndexerInjest.h"

constexpr static size_t INDEX_CHUNK_TARGET_SIZE = 1 * MB;

using scb::String;
using scb::Vector;

struct PostingListHeader
   {
      uint32_t postsOffset;
   };

struct ChunkHeader
   {
      uint32_t DocCount;
      uint32_t DictionaryOffset;   // from start of data[]
      uint32_t PostingListsOffset; // from start of data[]
      uint32_t EndOffset;          // from start of data[]
   };

struct ContiguousChunk
   {
   public:
      ChunkHeader header;
      char data[ INDEX_CHUNK_TARGET_SIZE - sizeof( header ) ];
   };

#endif // SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H
