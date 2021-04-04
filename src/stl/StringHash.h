#ifndef SCOOBLE_STRINGHASH
#define STRINGHASH

#include "String.h"

struct StringHash {
   size_t operator()(const scb::String& s) {
      const char* data = s.cstr();
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

#endif // SCOOBLE_STRINGHASH
