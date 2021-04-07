#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include "../stl/Pair.h"
#include "../stl/String.h"
#include "../stl/UniquePointer.h"
#include "../stl/Vector.h"

#include <atomic>
#include <cmath>
#include <fcntl.h>
#include <iostream>
#include <openssl/md5.h>
#include <sys/mman.h>
#include <sys/stat.h>

class Bloomfilter
   {
      struct HashResult
         {
            uint64_t result[ 2 ];
         };

      uint8_t *bits;
      int numHashes;
      uint64_t size;

      static HashResult hash( const scb::String &s )
         {
         auto *data = reinterpret_cast<uint8_t *>( s.data() );
         HashResult result{};
         MD5( data, s.length(), reinterpret_cast<uint8_t *>( result.result ) );
         return result;
         }

      [[nodiscard]] inline uint64_t kHash( uint8_t k,
                                           const HashResult &val ) const
         {
         return ( val.result[ 0 ] + k * val.result[ 1 ] ) % size;
         }

   public:
      Bloomfilter( const char *bloomFile )
         {

         int fd = open( bloomFile, O_RDWR );
         assert( fd != -1 && "failed to open() bloom file" );

         struct stat fileStat
            {
            };
         assert( fstat( fd, &fileStat ) != -1 &&
                 "failed to stat() bloom file" );

         void *blob =
            mmap( nullptr,                // system choose address for us
                  fileStat.st_size,       // length of mapping
                  PROT_READ | PROT_WRITE, // we want both read and write
                  MAP_SHARED,             // we want the change to persist
                  fd,                     // underlying file to use
                  0                       // the whole file from the beginning
            );
         if ( blob == MAP_FAILED )
            {
            std::cerr << strerror( errno ) << std::endl;
            exit( 1 );
            }
         if ( mlock( blob, fileStat.st_size ) != 0 )
            {
            std::cerr
               << "mlock() failed: " << strerror( errno ) << '\n'
               << "The success of this is critical to performance.\n"
               << "Try run `ulimit -l unlimited` first, or run this as root.\n";
            exit( 1 );
            }
         // Number of hashes is contained within the first value in mmapped
         // files as seen below.
         numHashes = *static_cast<uint32_t *>( blob );
         bits = static_cast<uint8_t *>( blob ) + sizeof( uint32_t );
         size = ( fileStat.st_size - sizeof( uint32_t ) ) * 8;
         }

      bool Insert( const scb::String &s )
         {
         auto val = hash( s );
         bool existed = true;
         for ( int i = 0;  i < numHashes;  ++i )
            {
            uint64_t slot = kHash( i, val );
            uint64_t bucket = slot / 8, bit = slot % 8;
            uint8_t old = __sync_fetch_and_or( bits + bucket, 1 << bit );
            existed = existed && ( old & ( 1 << bit ) );
            }
         return existed;
         }

      bool Contains( const scb::String &s )
         {
         auto val = hash( s );
         for ( int i = 0;  i < numHashes;  ++i )
            {
            uint64_t slot = kHash( i, val );
            uint64_t bucket = slot / 8, bit = slot % 8;
            if ( !( bits[ bucket ] & ( 1 << bit ) ) )
               return false;
            }

         return true;
         }
   };

#endif
