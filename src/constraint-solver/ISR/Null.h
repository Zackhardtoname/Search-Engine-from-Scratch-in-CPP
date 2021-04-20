//
// Created by youh on 4/15/21.
//

#ifndef SCOOBLE_SEARCH_NULL_H
#define SCOOBLE_SEARCH_NULL_H

#include "ISR.h"
#include <limits>

class Null : public ISR
   {
   public:
      Null( const ChunkHeader *chunkHeader ) : ISR( chunkHeader )
         {
         }

      void Next() override
         {
         }

      bool IsEOF() override
         {
         return true;
         }

      uint32_t Get() override
         {
         return -1;
         }

      void NextDoc() override
         {
         }

      void Seek( uint32_t post ) override
         {
         }

      float Frequency() override
         {
         return std::numeric_limits<float>::max();
         }

      uint32_t DocID() override
         {
         return -1;
         }
   };

#endif // SCOOBLE_SEARCH_NULL_H
