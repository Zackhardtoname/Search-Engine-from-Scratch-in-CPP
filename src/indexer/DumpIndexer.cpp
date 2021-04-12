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

int main( int argc, char **argv )
   {
   if ( argc != 2 )
      {
      std::cout << "usage: " << argv[ 0 ] << " [dump file]" << std::endl;
      return 1;
      }

   auto *curChunk = new IndexChunk();

   FILE *f = fopen( argv[ 1 ], "r" );
   for ( int i = 0;  ;  i++ )
      {
      char sizeBuf[ 4 ];
      switch ( fread( sizeBuf, 4, 1, f ) )
         {
         case 0:
            std::cout << "<eof>" << std::endl;
            return 0;
         case 1:
            break;
         default:
            assert( false && "corrupted: cannot read 4-byte dump header" );
         }
      int32_t lenMsg = *reinterpret_cast<int32_t *>( sizeBuf );
      String msg( lenMsg, '\0' );
      std::cout << "dumped msg: " << i << " len = " << lenMsg << std::endl;
      assert( fread( msg.data(), lenMsg, 1, f ) == 1 &&
              "corrupted: did not read in enough bytes" );

      size_t consumed = -1;
      auto ingest = Deserialize<IndexerIngest>( msg, &consumed );
      assert( consumed == lenMsg &&
              "corrupted: deserialization did not use all bytes" );
      std::cout << "ingest for url: " << ingest.Url << std::endl;

      curChunk->Ingest( ingest );
      if ( curChunk->EstSize >= INDEX_CHUNK_TARGET_SIZE )
         {
         // persist the old chunk
         // get current time
         time_t timestamp = std::time( nullptr );
         tm *timeRepr = std::gmtime( &timestamp );
         char timeStr[ 1000 ];
         strftime( timeStr, 1000, "%Y_%m_%d_%H_%M_%S_", timeRepr );
         String filename( timeStr );
         filename += RandAlphaString( 6 ) + ".index";

         curChunk->writeToDisk( filename.data() );

         std::cout << "written chunk to " << filename.cstr() << std::endl;
         delete curChunk;
         curChunk = new IndexChunk();
         }
      }
   }
