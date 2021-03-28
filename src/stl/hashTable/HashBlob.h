#pragma once

// HashBlob, a serialization of a HashTable into one contiguous
// block of memory, possibly memory-mapped to a HashFile.

// Nicole Hamilton  nham@umich.edu

#include <cassert>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "HashTable.h"

#define NullTerminatorSize 8
constexpr size_t SerialTupleBaseSize = 3 * sizeof( size_t );

using Hash = HashTable<const char *, size_t>;
using Pair = Tuple<const char *, size_t>;
using HashBucket = Bucket<const char *, size_t>;

static const size_t Unknown = 0;

size_t RoundUp( size_t length, size_t boundary )
   {
   // Round up to the next multiple of the boundary, which
   // must be a power of 2.

   static const size_t oneless = boundary - 1, mask = ~( oneless );
   return ( length + oneless ) & mask;
   }

struct SerialTuple
   {
      // This is a serialization of a HashTable< char *, size_t >::Bucket.
      // One is packed up against the next in a HashBlob.

      // Since this struct includes size_t and uint32_t members, we'll
      // require that it be sizeof( size_t ) aligned to avoid unaligned
      // accesses.

   public:
      // SerialTupleLength = 0 is a sentinel indicating
      // this is the last SerialTuple chained in this list.
      // (Actual length is not given but not needed.)

      size_t Length, Value;
      size_t HashValue;

      // The Key will be a C-string of whatever length.
      char Key[ Unknown ];

      static size_t BytesForBucket( const HashBucket *b )
         {
         return SerialTupleBaseSize + strlen(b->tuple.key);
         }

      // Calculate the bytes required to encode a HashBucket as a
      // SerialTuple.

      static size_t BytesRequired( const HashBucket *b )
         {
         size_t running_length = 0;

         // add up bytes required for each bucket in the chain
         while ( b )
            {
            running_length += BytesForBucket( b );
            b = b->next;
            }

         if ( running_length > 0 )
            running_length += NullTerminatorSize;

         return running_length;
         }

      // Write the HashBucket out as a SerialTuple in the buffer,
      // returning a pointer to one past the last character written.

      static char *Write( char *buffer, char *bufferEnd, const HashBucket *b )
         {

         while ( b )
            {

            size_t *size_t_buf = reinterpret_cast<size_t *>( buffer );

            // write member vars byte by byte to buffer
            *size_t_buf++ = BytesForBucket( b );
            *size_t_buf++ = b->tuple.value;
            *size_t_buf++ = b->hashValue;

            buffer = reinterpret_cast<char *>( size_t_buf );

            size_t i = 0;
            while ( b->tuple.key[ i ] )
               {
               *buffer++ = b->tuple.key[ i ];
               ++i;
               }

            b = b->next;
            }

         size_t *size_t_buf = reinterpret_cast<size_t *>( buffer );
         *size_t_buf++ = 0;
         buffer = reinterpret_cast<char *>( size_t_buf );

         return buffer; // this will be the start of buffer for the next call to
                        // Write
         }
   };

class HashBlob
   {
      // This will be a hash specifically designed to hold an
      // entire hash table as a single contiguous blob of bytes.
      // Pointers are disallowed so that the blob can be
      // relocated to anywhere in memory

      // The basic structure should consist of some header
      // information including the number of buckets and other
      // details followed by a concatenated list of all the
      // individual lists of tuples within each bucket.

   public:
      // Define a MagicNumber and Version so you can validate
      // a HashBlob really is one of your HashBlobs.

      size_t MagicNumber;
      size_t Version;
      size_t BlobSize;
      size_t NumberOfBuckets;
      size_t Buckets[ Unknown ];

      // The SerialTuples will follow immediately after.

      const SerialTuple *Find( const char *key ) const
         {
         if (!strcmp(key, "would")) std::cout << "testing: blob find on " << key << std::endl;
         // Search for the key k and return a pointer to the
         // ( key, value ) entry.  If the key is not found,
         // return nullptr.

         // 1. calculate hashValue -> TODO: need to add fnvHash code into this
         // file
         size_t hashVal = Hash::fnvHash( key, strlen( key ) );
         // 2. calculate index
         size_t index = hashVal % NumberOfBuckets;
         // 3. check Buckets to see if the key even exists
         size_t chainStart = Buckets[ index ];
         if (!strcmp(key, "would")) std::cout << "testing: blob find on chainStart" << std::endl;
         // If there's nothing at this bucket, return nullptr
         if ( chainStart == 0 )
            {
            return nullptr;
            }

         // chainAddress is used to read right from the hashBlob, it points to
         // the beginning of the blob
         const char *chainAddress =
            reinterpret_cast<const char *>( &MagicNumber ) + chainStart;
         size_t tupleLength = chainStart;

         if (!strcmp(key, "would")) std::cout << "testing: blob find on chainAddress" << std::endl;
         while ( true )
            {

            if (!strcmp(key, "would")) std::cout << "testing: blob find beginning loop" << std::endl;
            tupleLength = *chainAddress;

            // If we reach the end of the chain
            if ( tupleLength == 0 )
               return nullptr;

            // Need to increment by factors of 8 here since size_t is 8 bytes
            // and chainAddress is a char*
            size_t tupleVal = *( chainAddress + 8 );
            size_t tupleHashVal = *( chainAddress + 16 );

            if (!strcmp(key, "would")) std::cout << "testing: blob find part through loop tupleLength: " << tupleLength  << std::endl;
            // Subtract by 23 since we want the 24 bytes from the Length, Value,
            // and Hash Value removed from the overall length, yet we want one
            // extra character for a null char
            char tupleKey[ tupleLength - 23 ];

            // Read in the key from the hashBlob
            for ( size_t i = 24;  i < tupleLength;  ++i )
               {
               tupleKey[ i - 24 ] = *( chainAddress + i );
               }

            if (!strcmp(key, "would")) std::cout << "testing: blob find reading portion over" << std::endl;

            tupleKey[ tupleLength - 24 ] = '\0';
            auto k = reinterpret_cast<const SerialTuple *>( chainAddress );

            // If we found the key we were looking for
            if ( !strcmp( tupleKey, key ) )
               return reinterpret_cast<const SerialTuple *>( chainAddress );

            chainAddress += tupleLength;

            if (!strcmp(key, "would")) std::cout << "testing: blob find ending loop" << std::endl;
            }

         return nullptr;
         }

      static size_t BytesRequired( const Hash *hashTable )
         {
         // Calculate how much space it will take to
         // represent a HashTable as a HashBlob.

         size_t header_size = 4 * sizeof( size_t );
         size_t buckets_section_size =
            sizeof( size_t ) * ( hashTable->numberOfBuckets );
         size_t tuples_section_size = 0;

         // Sum up the space required for all serialtuples (including null
         // terminators)
         for ( int i = 0;  i < hashTable->numberOfBuckets;  ++i )
            {
            // Assume that BytesRequired accounts for separate chaining and
            // null terminator size
            tuples_section_size +=
               SerialTuple::BytesRequired( hashTable->buckets[ i ] );
            }

         return header_size + buckets_section_size + tuples_section_size;
         }

      // Write a HashBlob into a buffer, returning a
      // pointer to the blob.

      static HashBlob *Write( HashBlob *hb, size_t bytes,
                              const Hash *hashTable )
         {
         // TODO: assume Create must have been called before write?
         hb->MagicNumber = 0;
         hb->Version = 1;
         hb->BlobSize = bytes;
         hb->NumberOfBuckets = hashTable->numberOfBuckets;

         for ( size_t i = 0;  i < hb->NumberOfBuckets;  ++i )
            {
            *( hb->Buckets + i ) = 0;
            }

         char *startOfEmptyTupleMemory =
            reinterpret_cast<char *>( hb->Buckets + hb->NumberOfBuckets );

         for ( size_t i = 0;  i < hashTable->numberOfBuckets;  ++i )
            {
            if ( hashTable->buckets[ i ] )
               {
               // TODO: If hb->Buckets gets really big, it may make more sense
               // to make the offset from tupleMemory-1 instead of from the
               // beginning of the hashblob
               *( hb->Buckets + i ) = static_cast<size_t>(
                  startOfEmptyTupleMemory -
                  reinterpret_cast<char *>( &hb->MagicNumber ) );
               startOfEmptyTupleMemory = SerialTuple::Write(
                  startOfEmptyTupleMemory, startOfEmptyTupleMemory + bytes,
                  hashTable->buckets[ i ] );
               }
            }

         return hb;
         }

      // Create allocates memory for a HashBlob of required size
      // and then converts the HashTable into a HashBlob.
      // Caller is responsible for discarding when done.

      // (No easy way to override the new operator to create a
      // variable sized object.)

      static HashBlob *Create( const Hash *hashTable )
         {
         // TODO: use malloc() here based on BytesRequired
         // TODO: maybe avoid duplicate call to BytesRequired?
         size_t numBytes = BytesRequired( hashTable );
         return Write( reinterpret_cast<HashBlob *>( malloc( numBytes ) ),
                       numBytes, hashTable );
         }

      // Discard

      static void Discard( HashBlob *blob )
         {
         // TODO: this is effectively a destructor!
         // need to call free on the blob!
         free( reinterpret_cast<void *>( blob ) );
         }
   };

class HashFile
   {
   private:
      HashBlob *blob;
      int fd = 0;

      size_t FileSize( int f )
         {
         struct stat fileInfo;
         fstat( f, &fileInfo );
         return fileInfo.st_size;
         }

   public:
      const HashBlob *Blob()
         {
         return blob;
         }

      HashFile( const char *filename )
         {
         // Open the file for reading, map it, check the header,
         // and note the blob address.
         fd = open( filename, O_RDONLY );
         if ( fd == -1 )
            {
            std::cout << "Error: file not open\n";
            exit( 1 );
            }
         size_t sz = FileSize( fd );

         void *mapped = mmap( nullptr, sz, PROT_READ, MAP_SHARED, fd, 0 );
         if ( mapped == MAP_FAILED )
            {
            std::cout << "Error: mapping failed\n";
            exit( 1 );
            }

         // TODO "Check the header" -> this is not meaningful for the AG
         // I think this means check the version and magic number

         // TODO: HashBlob isn't correctly written into and read from a file,
         // needs to be fixed Interpret loaded data as HashBlob
         blob = reinterpret_cast<HashBlob *>( mapped );
         }

      HashFile( const char *filename, const Hash *hashtable )
         {
         // Open the file for write, map it, write
         // the hashtable out as a HashBlob, and note
         // the blob address.
         fd = open( filename, O_CREAT | O_RDWR, 0644 );
         if ( fd == -1 )
            {
            std::cout << "Error: file not open\n";
            exit( 1 );
            }

         // TODO: For the AG we don't need to call optimize, but we could consider
         // calling it here for our own code in case we forget to call optimize
         // before creating the HashBlob
         size_t sz = HashBlob::BytesRequired( hashtable );
         ftruncate( fd, sz );

         void *mapped =
            mmap( nullptr, sz, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0 );
         if ( mapped == MAP_FAILED )
            {
            std::cout << strerror( errno ) << std::endl;
            std::cout << "Error: mapping failed\n";
            exit( 1 );
            }
         mapped = HashBlob::Write( reinterpret_cast<HashBlob *>( mapped ), sz,
                                   hashtable );
         blob = reinterpret_cast<HashBlob *>( mapped );

         // HashBlob address saved
         }

      ~HashFile()
         {
         close( fd );
         }
   };
