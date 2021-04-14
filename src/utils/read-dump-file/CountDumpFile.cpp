//
// Created by youh on 3/16/21.
//

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

int main( int argc, char **argv )
   {
   if ( argc != 2 )
      {
      std::cout << "usage: " << argv[ 0 ] << " [dump file]" << std::endl;
      return 1;
      }

   FILE *f = fopen( argv[ 1 ], "r" );
   for ( int i = 0;  ;  i++ )
      {
      char sizeBuf[ 4 ];
      switch ( fread( sizeBuf, 1, 4, f ) )
         {
         case 0:
            std::cout << i << std::endl;
            return 0;
         case 4:
            break;
         default:
            std::cout << i << std::endl;
            std::cerr << "<corrupted>" << std::endl;
            return 1;
         }
      int32_t lenMsg = *reinterpret_cast<int32_t *>( sizeBuf );
      std::string msg( lenMsg, '\0' );

      if ( fread( msg.data(), lenMsg, 1, f ) != 1 )
         {
         std::cout << i << std::endl;
         std::cerr << "<corrupted>" << std::endl;
         return 1;
         }
      }
   }