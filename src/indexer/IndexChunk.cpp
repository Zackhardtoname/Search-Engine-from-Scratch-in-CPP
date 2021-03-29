//
// Created by youh on 3/14/21.
//

#include <cassert>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include "../stl/String.h"
#include "../stl/UniquePointer.h"
#include "../stl/HashTable.h"
#include "../stl/HashBlob.h"
#include "ContiguousChunk.h"
#include "IndexChunk.h"
#include "VLQ.h"

void IndexChunk::writeToDisk( const char *filename )
   {
   // Structure:
   // [Header]
   // [Document Table (URL/document idx -> document header)]
   // [URL List]
   // [Dictionary (word -> posting list offset)]
   // [Posting Lists]
   //
   // Header:
   // [i32 num of documents]
   // [i32 offset for posting list idx (the Dictionary)]
   // [i32 offset for posting lists]
   //
   // document table entry:
   // [i32 offset of first char of URL]
   // [i32 last post in the document]
   //
   // Each Posting List:
   // [i32 offset of post1]
   // [Sync table]
   // [post1 post2 ... postn]

   // Header
   ChunkHeader header{ .DocumentCount = Documents.size() };

   // Document table
   auto docTable = scb::make_unique<DocumentTableEntry[]>( Documents.size() );

   // make URL list, and fill in document table
   String urlList;
   for ( int i = 0;  i < Documents.size();  i++ )
      {
      docTable[ i ].UrlOffset = urlList.size();

      urlList += Documents[ i ].Url;
      urlList.pushBack( '\0' );
      }

   // dictionary of words -> posting list offset
   scb::HashTable<const char*, size_t> postingListOffsetMap;

   // make posting lists, and fill in the offset mapping
   String serializedPostingLists;
   for ( scb::Tuple<const char*, scb::Vector<uint64_t>> &tuple: PostingLists )
      {
      // tuple.key is the word
      postingListOffsetMap[ tuple.key ] = serializedPostingLists.size();
      serializedPostingLists += SerializePostingList( PostingLists[ tuple.key ] );
      }

   // make a hashblob out of the hashmap
   // TODO if possible, construct hashblob in the mapped location instead of having to memcpy
   scb::HashBlob *postingListOffsetHashblob = scb::HashBlob::Create( &postingListOffsetMap ) ;

   // We got everything, calculate the final size and offset
   header.DocumentTableOffset = sizeof( ChunkHeader );
   header.UrlListOffset = header.DocumentTableOffset +
                          Documents.size() * sizeof( DocumentTableEntry );
   header.WordToPostingListDictionaryOffset =
      header.UrlListOffset + urlList.size();
   header.PostingListsOffset = header.WordToPostingListDictionaryOffset +
                               postingListOffsetHashblob->Size();

   size_t chunkSize = header.PostingListsOffset + serializedPostingLists.size();

   // open the file for writing, and resize it
   int fd = open( filename, O_CREAT | O_RDWR );
   ftruncate( fd, chunkSize );

   // mmap it in
   auto *chunk = static_cast<uint8_t *>(
      mmap( nullptr,                // system choose address for us
            chunkSize,              // length of mapping
            PROT_READ | PROT_WRITE, // we want both read and write
            MAP_SHARED,             // we want the change to persist
            fd,                     // underlying file to use
            0                       // the whole file from the beginning
            ) );

   // write header
   memcpy( chunk, &header, sizeof( ChunkHeader ) );
   // write document table
   memcpy( chunk + header.DocumentTableOffset, docTable.get(),
           Documents.size() * sizeof( DocumentTableEntry ) );
   // write URL list
   memcpy( chunk + header.UrlListOffset, urlList.data(), urlList.size() );
   // write word -> posting list offset dictionary
   memcpy( chunk + header.WordToPostingListDictionaryOffset,
           postingListOffsetHashblob, postingListOffsetHashblob->Size() );
   // write posting lists
   memcpy( chunk + header.PostingListsOffset, serializedPostingLists.data(),
           serializedPostingLists.size() );

   // cleanup
   munmap( chunk, chunkSize );
   close( fd );
   }

void IndexChunk::Ingest( const IndexerIngest &ingest )
   {
   // If this document only has an URL, discard it
   if ( ingest.Words.empty() && ingest.TitleWords.empty() )
      return;

   // record document words
   for ( const String &word : ingest.Words )
      {
      auto &lst = PostingLists[ word.cstr() ];
      uint64_t post = ++nextPost;
      if ( lst.empty() )
         EstSize += VLQ::Length( post );
      else
         EstSize += VLQ::Length( post - lst.back() );
      lst.pushBack( post );
      }

   // record title words
   for ( const String &word : ingest.TitleWords )
      {
      auto &lst = PostingLists[ (String( "@" ) + word).cstr() ];
      uint64_t post = ++nextPost;
      if ( lst.empty() )
         EstSize += VLQ::Length( post );
      else
         EstSize += VLQ::Length( post - lst.back() );
      lst.pushBack( post );
      }

   // record document metadata
   Documents.pushBack( Document{ .Url = ingest.Url, .LastPost = nextPost } );

   // update estimated size
   EstSize += ingest.Url.length();
   EstSize += ingest.TitleWords.size() * 2;
   EstSize += ingest.Words.size() * 2;
   }

String SerializePostingList( const Vector<uint64_t> &lst )
   {
   // number of sync table entries = ceil ( number of posts / NUM_POSTS_COVERED)
   int nEntries = lst.size() / SyncTableEntry::NUM_POSTS_COVERED;
   if ( lst.size() % SyncTableEntry::NUM_POSTS_COVERED != 0 )
      nEntries++;

   // allocate the whole sync table, since we already know how large it is
   auto syncTable = scb::make_unique<SyncTableEntry[]>( nEntries );
   // of course, we start searching from the beginning for [lst[0],
   // lst[NUM_POSTS_COVERED])
   syncTable[ 0 ] = { .Offset = 0,
                      .Post = static_cast<uint32_t>( lst.front() ) };

   String encodedPosts;

   uint64_t prevPost = 0;
   int prevSyncSlot = 0;
   for ( int i = 0;  i < lst.size();  i++ )
      {
      uint64_t post = lst[ i ];

      int curSyncSlot = i / SyncTableEntry::NUM_POSTS_COVERED;
      if ( curSyncSlot != prevSyncSlot )
         {
         // write a new entry into the sync table
         syncTable[ curSyncSlot ] = { .Offset = static_cast<uint32_t>(
                                         encodedPosts.size() ),
                                      .Post = static_cast<uint32_t>( post ) };
         prevSyncSlot = curSyncSlot;
         }

      encodedPosts += VLQ::Encode( post - prevPost );
      prevPost = post;
      }

   // make header
   PostingListHeader header{
      .PostsOffset = static_cast<uint32_t>(
         sizeof( PostingListHeader ) + nEntries * sizeof( SyncTableEntry ) ),
      .NumSyncTableEntries = static_cast<uint32_t>( nEntries ),
      .NumPosts = lst.size()
   };

   // combine the three
   return String( reinterpret_cast<char *>( &header ),
                  sizeof( PostingListHeader ) ) +
          String( reinterpret_cast<char *>( syncTable.get() ),
                  nEntries * sizeof( SyncTableEntry ) ) +
          encodedPosts;
   }