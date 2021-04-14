#ifndef DOCMATCH_H
#define DOCMATCH_H

#include "../indexer/VLQ.h"
#include "../stl/wire.h"
#include <stdint.h>

struct DocMatch
   {
      double Rank;
      scb::String Url;
      scb::String Title;
      scb::String Snippet;
   };

template <> struct WireType<DocMatch>
   {
      static String ToWire( const DocMatch &val )
         {
         // = [rank][url]
         return VLQ::Encode( val.Rank ) + Serialize( val.Url );
         }

      static DocMatch FromWire( StringRef buf, size_t *consumed )
         {
         DocMatch match;
         size_t rankConsumed = -1;
         match.Rank = VLQ::Decode( buf, &rankConsumed );
         buf.Begin += rankConsumed;

         size_t urlConsumed = -1;
         match.Url = Deserialize<scb::String>( buf, &urlConsumed );

         *consumed = rankConsumed + urlConsumed;
         return match;
         }
   };

int cmpDocMatch( const void *pm1, const void *pm2 )
   {
   const auto *m1 = static_cast<const DocMatch *>( pm1 );
   const auto *m2 = static_cast<const DocMatch *>( pm2 );
   return ( m1->Rank - m2->Rank );
   }

#endif // DOCMATCH_H