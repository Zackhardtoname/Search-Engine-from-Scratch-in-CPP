//
// Created by youh on 4/12/21.
//

#include "../stl/String.h"
#include "../stl/utils.h"
#include "IndexChunk.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

using scb::String;

void Persist( IndexChunk **chunk )
   {
   // get current time
   time_t timestamp = std::time( nullptr );
   tm *timeRepr = std::gmtime( &timestamp );
   char timeStr[ 1000 ];
   strftime( timeStr, 1000, "%Y_%m_%d_%H_%M_%S_", timeRepr );
   String filename( timeStr );
   filename += RandAlphaString( 6 ) + ".chunk";

   ( *chunk )->writeToDisk( filename.data() );

   std::cout << "written chunk to " << filename.cstr() << std::endl;
   delete *chunk;
   *chunk = new IndexChunk();
   }

int main( int argc, char **argv )
   {
   if ( argc <= 2 )
      {
      std::cout << "usage: " << argv[ 0 ] << " [dump file] [target size in MB]"
                << std::endl;
      return 1;
      }

   long targetSize = INDEX_CHUNK_TARGET_SIZE;
   if ( argc >= 3 )
      {
      targetSize = atol( argv[ 2 ] );
      std::cout << "targeting dump size " << targetSize << " MB" << std::endl;
      targetSize *= MB;
      }
   assert( targetSize > 0 && "invalid target size" );

   auto *curChunk = new IndexChunk();

   FILE *f = fopen( argv[ 1 ], "r" );
   for ( int i = 0;  ;  i++ )
      {
      char sizeBuf[ 4 ];
      switch ( fread( sizeBuf, 4, 1, f ) )
         {
         case 0:
            Persist( &curChunk );
            std::cout << "<eof>" << std::endl;
            return 0;
         case 1:
            break;
         default:
            Persist( &curChunk );
            std::cerr << "corrupted: cannot read 4-byte dump header"
                      << std::endl;
            return 1;
         }
      int32_t lenMsg = *reinterpret_cast<int32_t *>( sizeBuf );
      String msg( lenMsg, '\0' );
      if ( fread( msg.data(), lenMsg, 1, f ) != 1 )
         {
         Persist( &curChunk );
         std::cerr << "corrupted: did not read in enough bytes" << std::endl;
         return 1;
         }

      size_t consumed = -1;
      auto ingest = Deserialize<IndexerIngest>( msg, &consumed );
      if ( consumed != lenMsg )
         {
         Persist( &curChunk );
         std::cerr << "corrupted: deserialization did not use all bytes"
                   << std::endl;
         return 1;
         }

      curChunk->Ingest( ingest );
      if ( curChunk->EstSize >= targetSize )
         Persist( &curChunk );
      }
   }
