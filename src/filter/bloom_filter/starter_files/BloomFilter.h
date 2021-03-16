#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H
#include <array>
#include <cmath>
#include <iostream>
#include "../../../stl/include/Unique_Pointer.h"
#include <openssl/md5.h>
#include <string.h>
#include <string>
#include <vector>
class Bloomfilter
   {
   public:
      Bloomfilter( int n, double fpr )
          : buff{ make_unique<unsigned char[]>( DIGEST_LENGTH ) }
         {
         bits.resize( long( n * abs( log( fpr ) ) / ( log( 2 ) * log( 2 ) ) ),
                      0 );
         num_hashes = int( bits.size() * log( 2 ) / n );
         }
      void insert( const std::string &s )
         {

         auto val = hash( s );
         for ( int i = 0;  i < num_hashes;  ++i )
            bits[ kHash( i, val.first, val.second, bits.size() ) ] = 1;
         }

      bool contains( const std::string &s )
         {
         auto val = hash( s );
         for ( int i = 0;  i < num_hashes;  ++i )
            if ( !bits[ kHash( i, val.first, val.second, bits.size() ) ] )
               return false;

         return true;
         }

   private:
      static constexpr size_t DIGEST_LENGTH = 16;
      // Add any private member variables that may be neccessary
      inline uint64_t kHash( uint8_t k, uint64_t A, uint64_t B, uint64_t sz )
         {
         return ( A + k * B ) % sz;
         }
      // looks sus af, but vector has hidden property that if it is vector
      // of bools, then it acts as a dynamic bitset. Since '98.
      std::vector<bool> bits;
      int num_hashes;

      std::pair<uint64_t, uint64_t> hash( const std::string &datum )
         {
         const unsigned char *const data =
            reinterpret_cast<const unsigned char *>( datum.data() );
         // unsigned char * D = new unsigned char [16]();
         MD5( data, datum.length(), buff.get() );
         // std::cout<<*(unsigned long long*)buff.get()<<"\t"<<*(unsigned long
         // long*)((buff.get())+8)<<"\n";
         return { *(unsigned long long *)buff.get(),
                  *(unsigned long long *)( ( buff.get() ) + 8 ) };
         }
      const unique_ptr<unsigned char[]> buff;
   };

#endif
