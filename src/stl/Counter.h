//
// Created by youh on 3/21/21.
//

#ifndef SCOOBLE_SEARCH_COUNTER_H
#define SCOOBLE_SEARCH_COUNTER_H

#include "../indexer/IndexChunk.h"
#include "String.h"
#include <unordered_map>

class Counter
   {
      std::unordered_map<scb::String, int, StringHash> data;

   public:
      int Get( const scb::String &x )
         {
         return data[ x ];
         }

      void Increment( const scb::String &x )
         {
         data[ x ]++;
         }
   };

#endif // SCOOBLE_SEARCH_COUNTER_H
