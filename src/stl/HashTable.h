// Simple hash table template.

// Nicole Hamilton  nham@umich.edu

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <cassert>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <string>
#include <unistd.h>
#include <vector>

namespace scb
   {

// Compare C-strings, return true if they are the same.
static bool CompareEqual( const char *L, const char *R )
   {
   while ( *L == *R )
      {
      if ( *L == '\0' && *R == '\0' )
         {
         return true;
         }
      L++, R++;
      }
   return false;
   }

template <typename Key, typename Value> class Tuple
   {
   public:
      Key key;
      Value value;

      Tuple( const Key &k, const Value v ) : key( k ), value( v )
         {
         }

      void print()
         {
         std::cout << key << "\t" << value << "\t";
         }
   };

template <typename Key, typename Value> class Bucket
   {
   public:
      Bucket *next = nullptr;
      size_t hashValue = 0;
      Tuple<Key, Value> tuple;

      Bucket( const Key &k, size_t h, const Value v )
          : tuple( k, v ), next( nullptr ), hashValue( h )
         {
         }

      void searchChainAndAdd( size_t &uniqueWords, Key k, size_t hashVal )
         {
         Bucket *b = this;
         while ( b )
            {
            // newBucket key matches existing
            if ( CompareEqual( b->tuple.key, k ) )
               {
               b->tuple.value++;
               return; // not inserted
               }
            // reached end of chain, append
            else if ( !b->next )
               {
               b->next = new Bucket( k, hashVal, 1 );
               uniqueWords++;
               return; // appended to chain
               }
            // keep going
            else
               {
               b = b->next;
               }
            }
         }

      void print()
         {
         Bucket *b = this;
         while ( b )
            {
            b->tuple.print();
            std::cout << b->hashValue << std::endl;
            b = b->next;
            }
         }
   };

template <typename Key, typename Value> class HashTable
   {
   private:
      static size_t fnvHash( const char *data, size_t length )
         {
         static const size_t OffsetBasis = 146959810393466560;
         static const size_t prime = 1099511628211ul;
         size_t hash = OffsetBasis;
         for ( size_t i = 0;  i < length;  ++i )
            {
            hash *= prime;
            hash ^= data[ i ];
            }
         return hash;
         }

      Bucket<Key, Value> **buckets;
      size_t numberOfBuckets;
      size_t numUniqueWords = 0;

      friend class Iterator;
      friend class HashBlob;

   public:
      Tuple<Key, Value> *Find( const Key k, const Value initialValue )
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) entry.  If the key is not already
         // in the hash, add it with the initial value.

         // hash key, traverse buckets chain if there, append if needed or
         // return

         size_t keyHash = fnvHash( k, strlen( k ) );
         size_t index = keyHash % numberOfBuckets;

         // search bucket chain for string
         Bucket<Key, Value> *bucket = buckets[ index ];

         if ( !bucket )
            {
            // if bucket is nullptr, nothing at this index yet
            buckets[ index ] =
               new Bucket<Key, Value>( k, keyHash, initialValue );
            ++numUniqueWords;
            return &buckets[ index ]->tuple;
            }
         else
            {
            // while we're not at the last bucket of chain
            Bucket<Key, Value> *prev = nullptr;
            while ( bucket )
               {
               // check if string matches
               if ( CompareEqual( bucket->tuple.key, k ) )
                  return &bucket->tuple;
               // otherwise, check next in chain
               else
                  {
                  prev = bucket;
                  bucket = bucket->next;
                  }
               }
            // kvp not in chain, so add to end of end (bucket)
            prev->next = new Bucket<Key, Value>( k, keyHash, initialValue );
            ++numUniqueWords;
            return &prev->next->tuple;
            }
         }

      Tuple<Key, Value> *Find( const Key k ) const
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) enty.  If the key is not already
         // in the hash, return nullptr.

         size_t keyHash = fnvHash( k, strlen( k ) );
         size_t index = keyHash % numberOfBuckets;

         // search bucket chain for string
         Bucket<Key, Value> *bucket = buckets[ index ];
         if ( !bucket )
            {
            // if bucket is nullptr, nothing at this index yet
            buckets[ index ] = new Bucket<Key, Value>( k, keyHash, Value() );
            //++numUniqueWords;
            return &buckets[ index ]->tuple;
            }
         else
            {
            // while we're not at the last bucket of chain
            Bucket<Key, Value> *prev = nullptr;
            while ( bucket )
               {
               // check if string matches
               if ( CompareEqual( bucket->tuple.key, k ) )
                  return &bucket->tuple;
               // otherwise, check next in chain
               else
                  {
                  prev = bucket;
                  bucket = bucket->next;
                  }
               }
            // kvp not in chain, so add to end of end (bucket)
            prev->next = new Bucket<Key, Value>( k, keyHash, Value() );
            //++numUniqueWords;
            return &prev->next->tuple;
            }
         return nullptr;
         }

      void Optimize( double loading = 1.5 )
         {
         // Modify or rebuild the hash table as you see fit
         // to improve its performance now that you know
         // nothing more is to be added.

         // allocate a new buckets of numUniqueWords * 1.5
         size_t newNumBuckets =
            static_cast<size_t>( ceil( numUniqueWords * loading ) );
         Bucket<Key, Value> **newBuckets =
            new Bucket<Key, Value> *[ newNumBuckets ]();
         // iterate through current buckets
         const std::function<void( Bucket<Key, Value> * )> reindexChain =
            [ & ]( Bucket<Key, Value> *bucket )
         {
               // start reindexing from end of chain
               if ( bucket->next )
                  reindexChain( bucket->next );

               // calculate new index
               size_t newIndex = bucket->hashValue % newNumBuckets;
               // insert bucket
               if ( newBuckets[ newIndex ] )
                  {
                  // just prepend since no more duplicates
                  bucket->next = newBuckets[ newIndex ];
                  newBuckets[ newIndex ] = bucket;
                  }
               else
                  {
                  // set bucket to location
                  newBuckets[ newIndex ] = bucket;
                  // ignore previous chain
                  newBuckets[ newIndex ]->next = nullptr;
                  }
         };
         // reindex with buckets[index]->hashValue % newBuckets
         for ( size_t index = 0;  index < numberOfBuckets;  ++index )
            {
            // reindex full bucket chain
            if ( buckets[ index ] )
               reindexChain( buckets[ index ] );
            }
         // replace buckets
         delete[] buckets;
         buckets = newBuckets;
         numberOfBuckets = newNumBuckets;
         }

      // Your constructor may take as many default arguments
      // as you like.

      HashTable()
         {
         numberOfBuckets = 1024;
         buckets = new Bucket<Key, Value> *[ numberOfBuckets ]();
         }

      HashTable( const scb::Vector<scb::String> &words )
         {
         numberOfBuckets = 8196;
         buckets =
            new Bucket<Key, Value> *[ numberOfBuckets ](); // zero-initialized
         // Populate buckets with words
         for ( auto &str : words )
            {
            size_t hashVal = fnvHash( str.cstr(), str.size() );
            size_t index = hashVal % numberOfBuckets;
            // if existing bucket, append new or increment value
            if ( buckets[ index ] )
               {
               buckets[ index ]->searchChainAndAdd( numUniqueWords, str.cstr(),
                                                    hashVal );
               }
            // otherwise, set bucket
            else
               {
               buckets[ index ] =
                  new Bucket<Key, Value>( str.cstr(), hashVal, 1 );
               numUniqueWords++;
               }
            }
         }

      void print()
         {
         Iterator it = begin();
         while ( it != end() )
            {
            std::cout << "index " << it.bucketIndex << "\t";
            it->print(); // print tuple
            std::cout << it.bucket->hashValue << std::endl;
            ++it;
            }
         std::cout << std::endl;
         // for (size_t i = 0; i < numberOfBuckets; ++i) {
         //   printf("index %d\n", i);
         //   if (buckets[i])
         //      buckets[i]->print();
         //   std::cout << std::endl;
         //   //sleep(1);
         //}
         std::cout << numUniqueWords << std::endl;
         }

      Value &operator[]( Key key )
         {
         return Find( key, Value() )->value;
         }

      ~HashTable()
         {
         const std::function<void( Bucket<Key, Value> * )> freeChain =
            [ & ]( Bucket<Key, Value> *bucket )
         {
               if ( bucket )
                  {
                  freeChain( bucket->next );
                  delete bucket;
                  }
         };
         for ( size_t index = 0;  index < numberOfBuckets;  ++index )
            {
            // delete chained buckets
            freeChain( buckets[ index ] );
            }
         delete[] buckets;
         }

      class Iterator
         {
         private:
            friend class HashTable;

            HashTable *hashTable;
            size_t bucketIndex;
            Bucket<Key, Value> *bucket;

            Iterator( HashTable *table, size_t bucket, Bucket<Key, Value> *b )
                : hashTable{ table }, bucketIndex{ bucket }, bucket{ b }
               {
               }

         public:
            Iterator() : Iterator( nullptr, 0, nullptr )
               {
               }

            Iterator(HashTable *table) : Iterator( table, 0, nullptr )
               {
                  for ( ;
                        bucketIndex < hashTable->numberOfBuckets && !bucket;  )
                     ++bucketIndex, bucket = hashTable->buckets[ bucketIndex ];

                  // If end of table
                  if ( bucketIndex >= hashTable->numberOfBuckets )
                     bucket = nullptr;
               }

            ~Iterator()
               {
               }

            Tuple<Key, Value> &operator*()
               {
               return bucket->tuple;
               }

            Tuple<Key, Value> *operator->() const
               {
               return &bucket->tuple;
               }

            // Prefix ++
            Iterator &operator++()
               {
               // If this chain has another bucket
               if ( bucket->next )
                  bucket = bucket->next;

               // If not, find the next chain with a filled bucket
               else
                  {
                  ++bucketIndex, bucket = hashTable->buckets[ bucketIndex ];

                  // Iterate to next bucket with contents (may be the current
                  // one)
                  for ( ;
                        bucketIndex < hashTable->numberOfBuckets && !bucket;  )
                     ++bucketIndex, bucket = hashTable->buckets[ bucketIndex ];
                  // If end of table
                  if ( bucketIndex >= hashTable->numberOfBuckets )
                     bucket = nullptr;
                  }
               return *this;
               }

            // Postfix ++
            Iterator operator++( int )
               {
               Iterator ret = *this;
               ++( *this );
               return ret;
               }

            bool operator==( const Iterator &rhs ) const
               {
               return bucket == rhs.bucket;
               }

            bool operator!=( const Iterator &rhs ) const
               {
               return bucket != rhs.bucket;
               }
         };

      Iterator begin()
         {
         return Iterator( this );
         }

      Iterator end()
         {
         // one past the end
         return Iterator();
         }
   };
   } // namespace scb

#endif