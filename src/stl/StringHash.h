// TODO: this is temporary. After we have our own hashmap implementation, ditch
// this
//  and use that instead.
#ifndef SCOOBLE_STRINGHASH
#define STRINGHASH

#include "String.h"

struct StringHash
   {
      std::size_t operator()( const scb::String &s ) const
         {
         return s.hash();
         }
   };

#endif // SCOOBLE_STRINGHASH