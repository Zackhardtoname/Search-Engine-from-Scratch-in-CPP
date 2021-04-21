//
// Created by youh on 4/15/21.
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
   if ( argc != 3 )
      {
      std::cout << "usage: " << argv[ 0 ] << " [dump file] [url]" << std::endl;
      return 1;
      }

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
            std::cerr << "corrupted: cannot read 4-byte dump header"
                      << std::endl;
            return 1;
         }
      int32_t lenMsg = *reinterpret_cast<int32_t *>( sizeBuf );
      String msg( lenMsg, '\0' );
      if ( fread( msg.data(), lenMsg, 1, f ) != 1 )
         {
         std::cerr << "corrupted: did not read in enough bytes" << std::endl;
         return 1;
         }

      size_t consumed = -1;
      auto ingest = Deserialize<IndexerIngest>( msg, &consumed );
      if ( consumed != lenMsg )
         {
         std::cerr << "corrupted: deserialization did not use all bytes"
                   << std::endl;
         return 1;
         }

      if ( ingest.Url == argv[ 2 ] )
         {
         std::cout << "===== word =====" << std::endl;
         for ( auto &word : ingest.Words )
            std::cout << word << ' ';
         std::cout << std::endl << "===== title =====" << std::endl;
         for ( auto &word : ingest.TitleWords )
            std::cout << word << ' ';
         std::cout << std::endl;

         return 0;
         }
      }
   }
