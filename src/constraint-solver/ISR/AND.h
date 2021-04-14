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
      // anchor returns true if succeed, or EOF returns false if not possible
      bool alignCompliments()
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

   public:
      explicit AND( scb::Vector<ISR *> cls, const ChunkHeader *chunkHeader )
          : clauses( std::move( cls ) ), ISR( chunkHeader )
         {
         assert( cls.size() >= 2 &&
                 "AND ISR combines at least two other ISRs" );
         }

      void Next() override
         {
         assert( !"AND::Next() is meaningless" );
         }

      void NextDoc() override
         {
         // return when either 1. one of the clauses EOF, or 2. find a match
         while ( true )
            {
            sortByFrequency();

            // seek anchor to the next document first
            clauses.front()->NextDoc();

            // give up if EOF
            if ( clauses.front()->IsEOF() )
               return;

            // seek anchor to be >= all compliments's
            uint32_t highestCompliment = 0;
            for ( int i = 1;  i < clauses.size();  i++ )
               {
               uint32_t post = clauses[ i ]->DocFirstPost();
               if ( post > highestCompliment )
                  highestCompliment = post;
               }
            clauses.front()->Seek( highestCompliment );

            // give up if EOF
            if ( clauses.front()->IsEOF() )
               return;

            // if successfully align the compliments in the same document
            // or EOF
            if ( alignCompliments() )
               return;
            }
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

         // give up if EOF
         if ( clauses.front()->IsEOF() )
            return;

         alignCompliments();
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
         return clauses.front()->DocFirstPost();
         }
   };

#endif // SCOOBLE_SEARCH_AND_H
