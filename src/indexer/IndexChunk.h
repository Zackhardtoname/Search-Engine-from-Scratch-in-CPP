//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_INDEXCHUNK_H
#define SCOOBLE_SEARCH_INDEXCHUNK_H

#include <cstddef>
#include <unordered_map>

#include "../stl/String.h"
#include "../stl/Vector.h"
#include "ContiguousChunk.h"
#include "IndexerInjest.h"

using scb::String;
using scb::Vector;

// TODO: this is temporary. After we have our own hashmap implementation, ditch
// this
//  and use that instead.
struct StringHash
   {
      std::size_t operator()( const String &s ) const
         {
         return s.hash();
         }
   };

class SyncEntry
   {
   public:
      unsigned seekOffsetDelta;
      unsigned postLocation;
   };

class PostingList
   {
   public:
      Vector<SyncEntry> syncTable;
      Vector<uint64_t> posts;
   };

class IndexChunk
   {

   public:
      Vector<String> urls;
      std::unordered_map<String, uint32_t, StringHash> postingListOffsets;
      Vector<PostingList> postingLists;
      size_t estSize = 0;

      void writeToDisk( const char *filename );
   };

#endif // SCOOBLE_SEARCH_INDEXCHUNK_H
