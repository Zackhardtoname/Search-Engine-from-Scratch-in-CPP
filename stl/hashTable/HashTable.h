// Simple hash table template.

// Nicole Hamilton  nham@umich.edu

#pragma once

#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdint>

using namespace std;


// You may add additional members or helper functions.


// Compare C-strings, return true if they are the same.

bool CompareEqual( const char *L, const char *R ){
    while (*L==*R)
        if (*(L++) = '\0')
            return true;
    return false;
};


template< typename Key, typename Value > class Tuple
   {
   public:
      Key key;
      Value value;

      Tuple( const Key &k, const Value v ) : key( k ), value( v )
         {
         }
   };


template< typename Key, typename Value > class Bucket
   {
   public:
      Bucket *next;
      uint32_t hashValue;
      Tuple< Key, Value > tuple;

      Bucket( const Key &k, uint32_t h, const Value v ) :
            tuple( k, v ), next( nullptr ), hashValue( h )
         {
         }
   };


template< typename Key, typename Value > class HashTable
   {
   private:

      // Your code here.

      Bucket< Key, Value > **buckets;
      size_t numberOfBuckets;

      friend class Iterator;
      friend class HashBlob;

   public:

      Tuple< Key, Value > *Find( const Key k, const Value initialValue )
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) entry.  If the key is not already
         // in the hash, add it with the initial value.
         
         // Your code here.

         return nullptr;
         }

      Tuple< Key, Value > *Find( const Key k ) const
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) enty.  If the key is not already
         // in the hash, return nullptr.

         // Your code here.

         return nullptr;
         }

      void Optimize( double loading = 1.5 )
         {
         // Modify or rebuild the hash table as you see fit
         // to improve its performance now that you know
         // nothing more is to be added.

         // Your code here.
         }


      // Your constructor may take as many default arguments
      // as you like.

      HashTable( )
         {
         // Your code here.
         }


      ~HashTable( )
         {
         // Your code here.
         }


      class Iterator
         {
         private:

            friend class HashTable;

            // Your code here.

            Iterator( HashTable *table, size_t bucket, Bucket<Key, Value> *b )
               {
               // Your code here.
               }

         public:

            Iterator( ) : Iterator( nullptr, 0, nullptr )
               {
               }

            ~Iterator( )
               {
               }

            Tuple< Key, Value > &operator*( )
               {
               // Your code here.
               }

            Tuple< Key, Value > *operator->( ) const
               {
               // Your code here.
               }

            // Prefix ++
            Iterator &operator++( )
               {
               // Your code here.
               }

            // Postfix ++
            Iterator operator++( int )
               {
               // Your code here.
               }

            bool operator==( const Iterator &rhs ) const
               {
               // Your code here.
               }

            bool operator!=( const Iterator &rhs ) const
               {
               // Your code here.
               }
         };

      Iterator begin( )
         {
         // Your code here.
         }

      Iterator end( )
         {
         // Your code here.
         }
   };
