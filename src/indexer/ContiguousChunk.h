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
#include "IndexerIngest.h"

constexpr static size_t INDEX_CHUNK_TARGET_SIZE = 1 * GB;

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

#endif // SCOOBLE_SEARCH_CONTIGUOUSCHUNK_H
