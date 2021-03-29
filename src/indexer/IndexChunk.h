//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_INDEXCHUNK_H
#define SCOOBLE_SEARCH_INDEXCHUNK_H

#include <cstddef>

#include "../stl/String.h"
#include "../stl/Vector.h"
#include "../stl/HashTable.h"
#include "ContiguousChunk.h"
#include "IndexerIngest.h"

using scb::String;
using scb::Vector;

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
      scb::HashTable<const char*, Vector<uint64_t> > PostingLists;

      void Ingest( const IndexerIngest &ingest );

      void writeToDisk( const char *filename );
   };

// This function delta-encodes the given posting list, and makes the accompanied
// sync table. In other words, it outputs: [PostingListHeader] [Sync Table]
// [Posts 1, 2, 3...]
String SerializePostingList( const Vector<uint64_t> &lst );

#endif // SCOOBLE_SEARCH_INDEXCHUNK_H
