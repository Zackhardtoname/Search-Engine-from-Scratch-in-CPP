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
#include "../stl/HashBlob.h"
#include "IndexerIngest.h"
#include "../constraint-solver/ISR.h"

constexpr static size_t INDEX_CHUNK_TARGET_SIZE = 1 * MB;

using scb::String;
using scb::Vector;

struct DocumentTableEntry
   {
      uint32_t UrlOffset; // from the start of URL list
      uint64_t LastPost;  // absolute last post of this document
   };

struct SyncTableEntry
   {
      // How many posts does one SyncTableEntry cover
      // Note: Currently set low for testing
      static constexpr int NUM_POSTS_COVERED = 500;

      // In this posting list, Post is recorded at Offset
      // (relative to start of the posting list)
      uint32_t Offset;
      uint32_t Post;
   };

struct PostingListHeader
   {
      // Offset from the beginning of the posting list to the first post
      // uint32_t PostsOffset;
      // Number of entries in the sync table
      uint32_t NumSyncTableEntries;
      // Number of posts
      uint32_t NumPosts;
   };

struct ChunkHeader
   {
      uint32_t DocumentCount;
      uint32_t DocumentTableOffset; // from start of the whole chunk
      uint32_t UrlListOffset;       // from start of the whole chunk
      uint32_t
         WordToPostingListDictionaryOffset; // from start of the whole chunk
      uint32_t PostingListsOffset;          // from start of the whole chunk
   };

class ContiguousChunk {
private:
   ChunkHeader header;

public:
   ChunkHeader *getHeader() {
      return reinterpret_cast<ChunkHeader*>(this);
   }

   WordISR getWordISR(scb::String &key) {

      const auto blob = reinterpret_cast<const scb::HashBlob*>( reinterpret_cast<char*>(this) + header.WordToPostingListDictionaryOffset );

      auto serialtuple = blob->Find(key);
      
      // If no results (serialtuple is nullptr)
      if(serialtuple == nullptr) {
         std::cout << "No list for key: " << key.cstr() << std::endl;
         return WordISR(nullptr);
      }

      unsigned internalOffset = serialtuple->Value;

      const char *cursor = reinterpret_cast<const char *>( this ) +
                           header.PostingListsOffset + internalOffset;

      const PostingListHeader *plHeader =
         reinterpret_cast<const PostingListHeader *>( cursor );

      std::cout << "\tHeader:" << std::endl;
      std::cout << "\t\tNumber of posts: " << plHeader->NumPosts
           << "\n\t\tNumber of Sync Table Entries: "
           << plHeader->NumSyncTableEntries << std::endl;

      // Read and print Sync Table
      std::cout << "\tSync Table:\tOffs:\tPost:" << std::endl;
      const SyncTableEntry *syncEntry =
         reinterpret_cast<const SyncTableEntry *>( plHeader + 1 );
      for ( size_t i = 0;  i < plHeader->NumSyncTableEntries;  ++i )
         {
         std::cout << "\t\t[" << i << "]\t" << syncEntry->Offset << "\t"
              << syncEntry->Post << std::endl;
         ++syncEntry;
         }

      cursor += sizeof(PostingListHeader) + ( plHeader->NumSyncTableEntries * sizeof(SyncTableEntry) );

      std::cout << "ISR For posting list: " << blob->Find(key)->GetKey().cstr() << std::endl;

      return WordISR(cursor);
   }


};

#endif // SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H
