// TopN.cpp
// Nicole Hamilton nham@umich.edu

// Given a hashtable, return a dynamically-allocated array
// of pointers to the top N pairs based on their values.
// Caller is responsible for deleting the array.

// Individual pairs are to be C-strings as keys and size_t's
// as values.

#include "TopN.h"
#include "HashTable.h"

using namespace std;

using Hash = HashTable<const char *, size_t>;
using Pair = Tuple<const char *, size_t>;

class PairArr
   {
   public:
      Pair **topN; // data array
   };

Pair **TopN( Hash *hashtable, int N )
   {
   // Find the top N pairs based on the values and return
   // as a dynamically-allocated array of pointers.  If there
   // are less than N pairs in the hash, remaining pointers
   // will be null.

   // allocate topN arr
   Pair **topN = new Pair *[ N ]();

   Hash::Iterator it = hashtable->begin();

   // insert first N
   for ( int i = 0;  i < N;  ++i )
      {
      topN[ i ] = &*it++;
      }

   // insertion sort first N
   for ( int i = 1;  i < N;  ++i )
      {
      Pair *kvp = topN[ i ];
      int index = i;
      while ( index > 0 && kvp->value > topN[ index - 1 ]->value )
         {
         topN[ index ] = topN[ index - 1 ];
         --index;
         }
      topN[ index ] = kvp;
      }

   // bubble sort as we iterate
   while ( it != hashtable->end() )
      {
      // insert tuple into array if larger than last element
      Pair *kvp = &*it++;
      if ( kvp->value > topN[ N - 1 ]->value )
         {
         topN[ N - 1 ] = kvp;
         // find proper spot
         int index = N - 1;
         while ( index > 0 && kvp->value > topN[ index - 1 ]->value )
            {
            topN[ index ] = topN[ index - 1 ];
            --index;
            }
         topN[ index ] = kvp;
         }
      }
   return topN;
   }
