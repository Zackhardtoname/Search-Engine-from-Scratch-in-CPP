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
#include "IndexerIngest.h"

using scb::String;
using scb::Vector;

// TODO: this is temporary. After we have our own hashmap implementation, ditch
//    this and use that instead.
struct StringHash
   {
      std::size_t operator()( const String &s ) const
         {
         return s.hash();
         }
   };

struct Document
   {
      String Url;
      uint64_t LastPost;
   };

class IndexChunk
   {
      uint64_t nextPost = 0;

   public:
      size_t EstSize = 0;

      Vector<Document> Documents;
      std::unordered_map<String, Vector<uint64_t>, StringHash> PostingLists;

      void Ingest( const IndexerIngest &ingest );

      void writeToDisk( const char *filename );
   };

// This function delta-encodes the given posting list, and makes the accompanied
// sync table. In other words, it outputs: [PostingListHeader] [Sync Table]
// [Posts 1, 2, 3...]
String SerializePostingList( const Vector<uint64_t> &lst );

#endif // SCOOBLE_SEARCH_INDEXCHUNK_H
