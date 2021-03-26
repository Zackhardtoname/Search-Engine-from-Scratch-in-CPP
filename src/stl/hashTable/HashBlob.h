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

#define NullTerminatorSize 8
constexpr size_t SerialTupleBaseSize = 3*sizeof(size_t);

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
      
      static size_t BytesForBucket( const HashBucket *b )
         {
         return SerialTupleBaseSize + sizeof( b->tuple.key );
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
         
         if(running_length > 0) running_length += NullTerminatorSize;

         return running_length;
         }

      // Write the HashBucket out as a SerialTuple in the buffer,
      // returning a pointer to one past the last character written.

      static char *Write( char *buffer, char *bufferEnd,
            const HashBucket *b )
         {

         // TODO: Should we write all the buckets in the chain?
         // ex. while loop, going through all buckets to b->next
         // All of the logic below would be same except changing
         // the if statement error check for bufferEnd exceeding numBytes
         // Also, might want to change BytesRequired to return the
         // numBytes required for ALL buckets in the chain (simple while loop again)
         // This is currently the implemented version, but is it correct?

         //size_t numBytes = BytesRequired( b );
         // error check numBytes TODO:?
         //if ( numBytes > (size_t)(bufferEnd - buffer) )
           // {
            // TODO: is this necessary?
            // what would we return here?
           // }
         // write each bucket in chain to buffer

         while ( b )
            {

            size_t *size_t_buf = reinterpret_cast<size_t*>(buffer);

            // write member vars byte by byte to buffer (TODO: is it definitely already allocated?)
            *size_t_buf++ = BytesForBucket(b);
            *size_t_buf++ = b->hashValue;
            *size_t_buf++ = b->tuple.value;

            buffer = reinterpret_cast<char*>(size_t_buf);

            for (size_t i = 0; i < sizeof( Key ); ++i )
               {
               *buffer++ = b->tuple.key[i];
               }
            b = b->next;
            }
         // TODO: write a sentinel SerialTuple at the end? could be more info on Piazza
         // essentailly a SerialTuple with Length = 0 should be written to the end
         // Should be an additional 24 bytes total? For the 3 size_t members? But all will be 0 value.
         for ( size_t i = 0; i < NullTerminatorSize; ++i )
            {
            *buffer++ = '0';
            }


         return buffer + 1; // this will be the start of buffer for the next call to Write
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

         size_t header_size =  4*sizeof(size_t);
         size_t buckets_section_size = sizeof(size_t)*(hashTable->numberOfBuckets);
         size_t tuples_section_size = 0;

         // Sum up the space required for all serialtuples (including null terminators)
         for(int i = 0; i < hashTable->numberOfBuckets; ++i)
            {
            // If the bucket isn't empty
            if ( hashTable->buckets[i] )
               {
               // Assume that BytesRequired accounts for separate chaining and null terminator size
               tuples_section_size += SerialTuple::BytesRequired(hashTable->buckets[i]);
               }
            }

         return header_size + buckets_section_size + tuples_section_size;
         }

      // Write a HashBlob into a buffer, returning a
      // pointer to the blob.

      static HashBlob *Write( HashBlob *hb, size_t bytes,
            const Hash *hashTable )
         {
         // TODO: assume Create must have been called before write?
         // TODO: not sure if need to allocate anything using bytes, should be malloc()ed by Create?
         // TODO: write each HashBucket in hashTable as a SerialTuple to hb
         hb->MagicNumber = 0;
         hb->Version = 1;
         hb->BlobSize = bytes;
         hb->NumberOfBuckets = hashTable->numberOfBuckets;
         
         for ( size_t i = 0; i < hb->NumberOfBuckets; ++i )
            {
            *( hb->Buckets + i ) = 0;
            }

         char *tupleMemory = reinterpret_cast<char *>(hb->Buckets) + hashTable->numberOfBuckets;

         for ( size_t i = 0; i < hashTable->numberOfBuckets; ++i )
            {
            if ( hashTable->buckets[ i ] ) 
               {
               // TODO: If hb->Buckets gets really big, it may make more sense to make the offset from 
               // tupleMemory-1 instead of from the beginning of the hashblob
               *( hb->Buckets + i ) = static_cast<size_t>(tupleMemory - reinterpret_cast<char*>(hb));
               tupleMemory = SerialTuple::Write( tupleMemory, tupleMemory + bytes, hashTable->buckets [ i ] );
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

         return reinterpret_cast<HashBlob*>(malloc(BytesRequired(hashTable)));
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
         int fd = open(filename, O_RDONLY);
         if (fd == -1)
            {
            std::cout << "Error: file not open\n";
            exit(1);
            }
         size_t sz = FileSize(fd);

         void* mapped = mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0);
         if (mapped == MAP_FAILED)
            {
            std::cout << "Error: mapping failed\n";
            exit(1);
            }

         // TODO "Check the header"
         // I think this means check the version and magic number
         
         // Interpret loaded data as HashBlob
         blob = reinterpret_cast<HashBlob*>(mapped);

         }

      HashFile( const char *filename, const Hash *hashtable )
         {
         // Open the file for write, map it, write
         // the hashtable out as a HashBlob, and note
         // the blob address.
         int fd = open(filename, O_WRONLY);
         if (fd == -1)
            {
            std::cout << "Error: file not open\n";
            exit(1);
            }

         // Optimize the hastable memory before storing it as a blob
         // TODO: For the AG we don't need to call this, but we could consider calling it here
         // for our own code in case we forget to call optimize before creating the HashBlob
         //hashtable->Optimize();

         size_t sz = HashBlob::BytesRequired(hashtable);

         void* mapped = mmap(nullptr, sz, PROT_WRITE, MAP_PRIVATE, fd, 0);
         if (mapped == MAP_FAILED)
            {
            std::cout << "Error: mapping failed\n";
            exit(1);
            }

         // TODO: do we need both create and write? Or can we just Create() directly into mapped*
         blob = HashBlob::Create(hashtable);
         
         // write HashBlob to memory
         blob = HashBlob::Write(reinterpret_cast<HashBlob*>(mapped), sz, hashtable);

         // HashBlob address saved
         }

      ~HashFile( )
         {
         // deallocate from addr blob to addr blob + numBytes -> TODO: save numBytes as member?
         // TODO: probably want to close the file here instead, save the fd
         }
   };
