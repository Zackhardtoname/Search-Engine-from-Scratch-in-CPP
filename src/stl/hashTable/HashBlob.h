#pragma once

// HashBlob, a serialization of a HashTable into one contiguous
// block of memory, possibly memory-mapped to a HashFile.

// Nicole Hamilton  nham@umich.edu

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>

#include "HashTable.h"


using Hash = HashTable< const char *, size_t >;
using Pair = Tuple< const char *, size_t >;
using HashBucket = Bucket< const char *, size_t >;


static const size_t Unknown = 0;


size_t RoundUp( size_t length, size_t boundary )
   {
   // Round up to the next multiple of the boundary, which
   // must be a power of 2.

   static const size_t oneless = boundary - 1,
      mask = ~( oneless );
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

      // Calculate the bytes required to encode a HashBucket as a
      // SerialTuple.

      static size_t BytesRequired( const HashBucket *b )
         {
         Value = b->tuple.value;
         HashValue = b->hashValue;
         Key = b->tuple.key;
         return Length = RoundUp( sizeof( Value ) + sizeof( HashValue ) + sizeof( Key ), 8);
         }

      // Write the HashBucket out as a SerialTuple in the buffer,
      // returning a pointer to one past the last character written.

      static char *Write( char *buffer, char *bufferEnd,
            const HashBucket *b )
         {
         size_t numBytes = BytesRequired( b );
         // error check numBytes TODO:?
         if ( numBytes > (size_t)(bufferEnd - buffer) )
            {
            // TODO: is this necessary?
            // what would we return here?
            }
         // write member vars byte by byte to buffer (TODO: is it definitely already allocated?)
         for ( size_t i = 0; i < sizeof( Length ); ++i )
            {
            *buffer++ = ((char*)&Length)[i];
            }
         for ( size_t i = 0; i < sizeof( HashValue ); ++i )
            {
            *buffer++ = ((char*)&HashValue)[i];
            }
         for ( size_t i = 0; i < sizeof( Value ); ++i )
            {
            *buffer++ = ((char*)&Value)[i];
            }
         for (size_t i = 0; i < sizeof( Key ); ++i )
            {
            *buffer++ = Key[i];
            }
         return buffer + 1;
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

      size_t MagicNumber,
         Version,
         BlobSize,
         NumberOfBuckets,
         Buckets[ Unknown ];

      // The SerialTuples will follow immediately after.


      const SerialTuple *Find( const char *key ) const
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) entry.  If the key is not found,
         // return nullptr.

         // Your code here.

         return nullptr;
         }


      static size_t BytesRequired( const Hash *hashTable )
         {
         // Calculate how much space it will take to
         // represent a HashTable as a HashBlob.

         // Need space for the header + buckets +
         // all the serialized tuples.

         // Your code here.

         return 0;
         }

      // Write a HashBlob into a buffer, returning a
      // pointer to the blob.

      static HashBlob *Write( HashBlob *hb, size_t bytes,
            const Hash *hashTable )
         {
         // TODO: not sure if need to allocate anything using bytes, should be malloc()ed by Create?
         // TODO: write each HashBucket in hashTable as a SerialTuple to hb

         return nullptr;
         }

      // Create allocates memory for a HashBlob of required size
      // and then converts the HashTable into a HashBlob.
      // Caller is responsible for discarding when done.

      // (No easy way to override the new operator to create a
      // variable sized object.)

      static HashBlob *Create( const Hash *hashTable )
         {
         // TODO: use malloc() here based on BytesRequired

         return nullptr;
         }

      // Discard

      static void Discard( HashBlob *blob )
         {
         // TODO: not exactly sure what to do here, just remove from memory?
            // write updates to file? remove blob from file?
         }
   };

class HashFile
   {
   private:

      HashBlob *blob;

      size_t FileSize( int f )
         {
         struct stat fileInfo;
         fstat( f, &fileInfo );
         return fileInfo.st_size;
         }

   public:

      const HashBlob *Blob( )
         {
         return blob;
         }

      HashFile( const char *filename )
         {
         // Open the file for reading, map it, check the header,
         // and note the blob address.
         int fd = open((char*)filename, O_RDONLY);
         if (fd == -1)
            {
            std::cout << "Error: file not open\n";
            exit(1);
            }
         size_t sz = FileSize(fd);

         char* mapped = (char*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0);
         if (mapped == MAP_FAILED)
            {
            std::cout << "Error: mapping failed\n";
            exit(1);
            }

         // TODO: check header, not sure what we are supposed to do here?

         // TODO: is this the correct way to save the blob address?
         blob = (HashBlob*)mapped; // TODO: should this instead start at the bytes after the header?
         }

      HashFile( const char *filename, const Hash *hashtable )
         {
         // Open the file for write, map it, write
         // the hashtable out as a HashBlob, and note
         // the blob address.
         int fd = open((char*)filename, O_WRONLY);
         if (fd == -1)
            {
            std::cout << "Error: file not open\n";
            exit(1);
            }
         size_t sz = FileSize(fd);

         char* mapped = (char*)mmap(nullptr, sz, PROT_WRITE, MAP_PRIVATE, fd, 0);
         if (mapped == MAP_FAILED)
            {
            std::cout << "Error: mapping failed\n";
            exit(1);
            }
         char* begin = mapped;

         // create HashBlob from given HashTable
         size_t numBytes = BytesRequired(hashtable);
         blob = Create(hashtable);
         
         // write HashBlob to memory
         blob = Write(blob, numBytes, hashtable);

         // HashBlob address saved
         }

      ~HashFile( )
         {
         // deallocate from addr blob to addr blob + numBytes -> TODO: save numBytes as member?
         }
   };
