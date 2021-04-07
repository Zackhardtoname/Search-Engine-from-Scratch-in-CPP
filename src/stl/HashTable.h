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
#include <unistd.h>

namespace scb
   {

template <typename Key, typename Value> class Tuple
   {
   public:
      Key key;
      Value value;

      Tuple( const Key k, const Value v ) : key( k ), value( v )
         {
         }

      void print()
         {
         std::cout << key << "\t" << value << "\n";
         }
   };

template <typename Key, typename Value> class Bucket
   {
   public:
      Bucket *next = nullptr;
      size_t hashValue = 0;
      Tuple<Key, Value> tuple;

      Bucket( const Key k, const Value &v, const size_t h )
          : tuple( k, v ), next( nullptr ), hashValue( h )
         {
         }

      // inserts into chain at this bucket
      // if key=k not found, add to end of chain
      // if key=k found, do not add value
      bool insert( const Key k, const Value &v, const size_t hashVal,
                   size_t &numUniqueWords )
         {
         Bucket *b = this;
         while ( b )
            {
            // newBucket key matches existing
            if ( b->tuple.key == k )
               {
               return false; // not inserted
               }
            // reached end of chain, append
            else if ( !b->next )
               {
               b->next = new Bucket( k, v, hashVal );
               ++numUniqueWords;
               return true; // appended to chain
               }
            // keep going
            else
               {
               b = b->next;
               }
            }

         assert( false && "should never run" );
         return false;
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

template <typename Key, typename Value, typename Hash> class HashTable
   {
   private:
      Bucket<Key, Value> **buckets;
      size_t numBuckets;
      size_t numUniqueWords = 0;
      Hash hash;

      friend class Iterator;
      friend class HashBlob;

   public:
      // insert new bucket of key=k value=v
      // returns true if inserted, false if not inserted (already exists)
      bool insert( const Key k, const Value &v )
         {
         size_t keyHash = hash( k );
         size_t index = keyHash % numBuckets;

         Bucket<Key, Value> *bucket = buckets[ index ];
         // if nothing at this bucket, insert new bucket here
         if ( !bucket )
            {
            buckets[ index ] = new Bucket<Key, Value>( k, v, keyHash );
            ++numUniqueWords;
            return true;
            }

         // otherwise insert into chain if not already there
         if ( find( k, index ) == end() )
            {
            buckets[ index ]->insert( k, v, keyHash, numUniqueWords );
            return true;
            }

         // otherwise already in table
         return false;
         }

      // get value for bucket associated with key=k
      // if not in table, create new bucket with default value
      Value &operator[]( const Key k )
         {
         size_t keyHash = hash( k );
         size_t index = keyHash % numBuckets;

         Bucket<Key, Value> *bucket = buckets[ index ];
         // if nothing at this bucket, insert new bucket here
         if ( !bucket )
            {
            buckets[ index ] = new Bucket<Key, Value>( k, Value(), keyHash );
            ++numUniqueWords;
            }

         // otherwise insert into chain if not already there
         if ( find( k, index ) == end() )
            {
            buckets[ index ]->insert( k, Value(), keyHash, numUniqueWords );
            }
         // otherwise already in table
         return buckets[ index ]->tuple.value;
         }

      void Optimize( double loading = 1.5 )
         {
         // Modify or rebuild the hash table as you see fit
         // to improve its performance now that you know
         // nothing more is to be added.

         // allocate a new buckets
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
         for ( size_t index = 0;  index < numBuckets;  ++index )
            {
            // reindex full bucket chain
            if ( buckets[ index ] )
               reindexChain( buckets[ index ] );
            }
         // replace buckets
         delete[] buckets;
         buckets = newBuckets;
         numBuckets = newNumBuckets;
         }

      // Your constructor may take as many default arguments
      // as you like.

      HashTable()
         {
         numBuckets = 8196;
         buckets = new Bucket<Key, Value> *[ numBuckets ]();
         }

      void print()
         {
         for ( size_t i = 0;  i < numBuckets;  ++i )
            {
            std::cout << "INDEX " << i << std::endl;
            if ( buckets[ i ] )
               buckets[ i ]->print();
            std::cout << std::endl;
            //           sleep(1);
            }
         std::cout << numUniqueWords << std::endl;
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
         for ( size_t index = 0;  index < numBuckets;  ++index )
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

         public:
            Iterator( HashTable *table, Bucket<Key, Value> *b, size_t idx )
                : hashTable{ table }, bucket{ b }, bucketIndex{ idx }
               {
               }

            Iterator() : Iterator( nullptr, nullptr, 0 )
               {
               }

            Iterator( HashTable *table ) : Iterator( table, nullptr, 0 )
               {
               for ( ;  bucketIndex < hashTable->numBuckets && !bucket;  )
                  ++bucketIndex, bucket = hashTable->buckets[ bucketIndex ];

               // If end of table
               if ( bucketIndex >= hashTable->numBuckets )
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
                  for ( ;  bucketIndex < hashTable->numBuckets && !bucket;  )
                     ++bucketIndex, bucket = hashTable->buckets[ bucketIndex ];
                  // If end of table
                  if ( bucketIndex >= hashTable->numBuckets )
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

      // Search for the key k and return an Iterator to the bucket.
      // If the key is not already in the table, return nullptr.
      Iterator find( const Key k, size_t index = SIZE_MAX )
         {
         if ( index == SIZE_MAX )
            index = hash( k ) % numBuckets;

         // search bucket chain for string
         Bucket<Key, Value> *bucket = buckets[ index ];
         if ( bucket )
            {
            // while we're not at the last bucket of chain
            while ( bucket )
               {
               // check if string matches
               if ( bucket->tuple.key == k )
                  return Iterator( this, bucket, index );
               // otherwise, check next in chain
               else
                  {
                  bucket = bucket->next;
                  }
               }
            }
         // not found if bucket nullptr || not found in chain
         return end();
         }
   };

   } // namespace scb

#endif
