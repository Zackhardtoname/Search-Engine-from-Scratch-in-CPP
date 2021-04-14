//
// Created by youh on 4/13/21.
//

#ifndef SCOOBLE_SEARCH_AND_H
#define SCOOBLE_SEARCH_AND_H

#include "../../stl/Vector.h"
#include "ISR.h"

// An AND ISR. A little bit special in that it only returns once per document.
// The rarest clause (with lowest frequency) is called the anchor, and the rest
// called the compliments.
class AND : public ISR
   {
      scb::Vector<ISR *> clauses;

      // Try to find matches for the compliments in the same document as the
      // anchor. Returns true if succeed, or EOF returns false if not possible.
      bool alignComplimentsInSameDoc()
         {
         // trying to match in anchor's clause's DocID, which spans [docBegin,
         // docEnd]
         ISR *anchor = clauses[ 0 ];
         uint32_t docBegin = anchor->DocFirstPost();
         uint32_t docEnd = chunk.LastPostTable[ anchor->DocID() ];

         // for the compliments, try to find a match in the document
         for ( int i = 1;  i < clauses.size();  i++ )
            {
            clauses[ i ]->Seek( docBegin );
            // give up if EOF
            if ( clauses[ i ]->IsEOF() )
               return true;

            // fail if no match in this document
            // (we cannot touch the anchor)
            if ( clauses[ i ]->Get() > docEnd )
               return false;
            }

         // if all clauses find a docBegin <= match <= docEnd, succeed
         return true;
         }

      void sortByFrequency()
         {
         // TODO: sort clauses by increasing frequency
         }

      // try to align the compliments to find a match based on the current
      // anchor, if not possible, move the anchor forward and repeat.
      void nextMatch()
         {
         while ( true )
            {
            // give up if anchor already
            if ( clauses.front()->IsEOF() )
               return;

            // Return if successfully align the compliments in the same
            // document, or EOF.
            if ( alignComplimentsInSameDoc() )
               return;

            // seek anchor to be >= all compliments's.
            uint32_t highestCompliment = 0;
            for ( int i = 1;  i < clauses.size();  i++ )
               {
               // If one of the compliments EOF
               if ( clauses[ i ]->IsEOF() )
                  return;

               uint32_t post = clauses[ i ]->DocFirstPost();
               if ( post > highestCompliment )
                  highestCompliment = post;
               }
            clauses.front()->Seek( highestCompliment );
            }
         }

   public:
      explicit AND( scb::Vector<ISR *> cls, const ChunkHeader *chunkHeader )
          : clauses( std::move( cls ) ), ISR( chunkHeader )
         {
         assert( clauses.size() >= 2 &&
                 "AND ISR combines at least two other ISRs" );

         sortByFrequency();

         nextMatch();
         }

      void Next() override
         {
         assert( !"AND::Next() is meaningless" );
         }

      void NextDoc() override
         {
         // seek anchor to the next document first
         clauses.front()->NextDoc();

         nextMatch();
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
         clauses.front()->DocFirstPost();
         }

      void Seek( uint32_t post ) override
         {
         sortByFrequency();

         clauses.front()->Seek( post );

         nextMatch();
         }

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
         return clauses.front()->DocID();
         }
   };

#endif // SCOOBLE_SEARCH_AND_H
