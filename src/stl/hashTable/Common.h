#pragma once
// Common code used by the various hashing sample applications.

// Nicole Hamilton  nham@umich.edu

#include "HashTable.h"
#include <string>
#include <vector>

using namespace std;

// You may define additional helper routines here and in
// Common.cpp.

// Compare C-strings, return true if they are the same.

// bool CompareEqual( const char *L, const char *R );

// Build a HashTable of strings and numbers of occurrences, given a vector
// of strings representing the words.  You may assume the vector and the
// strings will not be changed during the lifetime of the Hash.

// You may add additional default parameters.

// Caller is responsible for deleting the Hash.
struct StringHash {
   size_t operator()(std::string s) {
      const char* data = s.c_str();
      size_t length = strlen(data);
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
};

scb::HashTable<std::string, size_t, StringHash> *BuildHashTable( const vector<string> &words );

// Collect words read from a file specified on the command line
// as either individual word or whole lines in a vector of
// strings.

void CollectWordsIn( int argc, char **argv, vector<string> &words );

// -v (verbose) command line option.

extern bool optVerbose;
