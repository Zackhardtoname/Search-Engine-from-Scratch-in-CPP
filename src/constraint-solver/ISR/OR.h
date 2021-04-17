//
// Created by mjwolf on 4/13/21.
//

#ifndef SCOOBLE_SEARCH_OR_H
#define SCOOBLE_SEARCH_OR_H

#include "../../stl/Vector.h"
#include "ISR.h"

// An OR ISR. Note that all the component ISR's are always ahead of 
// the position maintainedin the OR ISR's state
class OR : public ISR
   {
      friend class QueryableChunk;

      scb::Vector<ISR *> clauses;
      uint32_t currentPost;
      uint32_t docID;

   public:
      explicit OR( scb::Vector<ISR *> cls, const ChunkHeader *chunkHeader )
          : clauses( std::move( cls ) ), ISR( chunkHeader ),
          currentPost( 0 )
         {
         assert( clauses.size() >= 2 &&
                 "OR ISR combines at least two other ISRs" );

         }

      // Move the nearest term to its next
      void Next() override
         {
            // Find the nearest component ISR:
            uint32_t minDistance = UINT32_MAX;
            uint32_t nearestIsrIndex;
            for( size_t i = 0; i < clauses.size(); ++i ) {
               uint32_t distance = clauses[i]->Get() - currentPost;
               
               if ( distance < minDistance ) {
                  minDistance = distance;
                  nearestIsrIndex = i;
               }
            }

            // Set the position to that of the nearest ISR
            currentPost = clauses[nearestIsrIndex]->Get();
            docID = clauses[nearestIsrIndex]->DocID();

            // Advance the nearest ISR
            clauses[nearestIsrIndex]->Next();
         }

      void NextDoc() override
         {
            // Seek all to first occurence past the end of the current document
            // Seek calls Next()
            Seek(chunk.LastPostTable[ docID ]);
         }

      bool IsEOF() override
         {
         for ( ISR *clause : clauses )
            if ( clause->IsEOF() )
               return true;

         return false;
         }

      uint32_t Get() override
         {
         return currentPost;
         }

      void Seek( uint32_t post ) override
         {
         for ( ISR* i : clauses )
            {
               i->Seek(post);
            }

         // Advance to first occurence after the seek input post
         Next();
         }

      // ? Does this need to be changed for the OR ISR?
      float Frequency() override
         {
         // TODO: a priori frequency is probably too naive
         float freq = 1;
         for ( auto *clause : clauses )
            freq *= clause->Frequency();
         return freq;
         }

      uint32_t DocID() override
         {
         return docID;
         }
   };

#endif // SCOOBLE_SEARCH_OR_H
