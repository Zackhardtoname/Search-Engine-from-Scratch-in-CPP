//
// Created by youh on 3/14/21.
//

#include <cassert>
#include <ctime>
#include <iostream>

#include "../stl/String.cpp"
#include "../stl/String.h"
#include "ContiguousChunk.h"
#include "IndexChunk.h"
#include "VLQ.h"

void IndexChunk::writeToDisk( const char *filename )
   {
   // Structure:
   // [Header] [URL List] [Dictionary (word -> posting list offset)] [Posting
   // Lists]
   //
   // Header:
   // [i32 num of documents] [i32 offset for posting list idx] [i32 offset for
   // posting lists]
   //
   // Each Posting List:
   // [i32 offset of first post]

   // generate filename from current time
   time_t timestamp = std::time( nullptr );
   tm *timeRepr = std::gmtime( &timestamp );
   // strftime(filename, 1000, "%Y_%m_%d_%H_%M_%S.chunk", timeRepr);

   // Open chunk file
   FILE *f = fopen( filename, "w" ); // append
   long curSize = 0;

   // Create contiguous chunk for writing
   ContiguousChunk *chunk = new ContiguousChunk();

   chunk->header.DocCount = urls.size();
   char *cursor = chunk->data;

   // Copy the URLs into the URL list
   for ( int i = 0;  i < urls.size();  ++i )
      {
      strcpy( cursor, urls[ i ].cstr() );
      // Increase pos to next string pos, leaving space for terminator
      cursor += urls[ i ].length() + 1;
      }

   // TODO hash blob
   chunk->header.DictionaryOffset = cursor - chunk->data;
   strcpy( cursor, "________TODO________" );
   cursor += sizeof( "________TODO________" ) - 1;

   // Copy URLS into the posting list section of the data
   chunk->header.PostingListsOffset = cursor - chunk->data;
   // For each posting list

   int plcount = 0;

   for ( const PostingList &pl : postingLists )
      {
      PostingListHeader plHeader { 0 };
      // TODO write synchronization tables
      // TODO correct offset
      memcpy( cursor, &plHeader, sizeof( PostingListHeader ) );
      cursor += sizeof( PostingListHeader );

      // For each post
      std::cout << "Writing posting list " << plcount << "\n";

      for ( uint64_t post : pl.posts )
         {
         String encoded = VLQ::Encode( post );

         memcpy( cursor, encoded.cstr(), encoded.length() );
         cursor += encoded.length();
         }

      // Add a sentinel at the end of each PL
      strcpy( cursor, "\0" ); // This line may not be necessary, as bytes should
                              // be 0x00 by default.
      ++cursor;
      ++plcount;
      }

   chunk->header.EndOffset = cursor - chunk->data;

   // Write contiguous chunk to file
   char *chunkBuffer = reinterpret_cast<char *>( chunk );
   size_t written = fwrite( chunkBuffer, sizeof( ContiguousChunk ), 1, f );
   std::cout << written << " chunks written." << std::endl;
   fclose( f );
   }
