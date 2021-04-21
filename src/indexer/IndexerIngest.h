//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_INDEXERINGEST_H
#define SCOOBLE_SEARCH_INDEXERINGEST_H

#include "../stl/String.h"
#include "../stl/Vector.h"
#include "../stl/wire.h"

using scb::String;
using scb::Vector;

struct IndexerIngest
   {
      Vector<String> Words;
      Vector<String> TitleWords;
      String Url;
   };

template <> struct WireType<IndexerIngest>
   {
      static String ToWire( const IndexerIngest &val )
         {
         return Serialize( val.Words ) + Serialize( val.TitleWords ) +
                Serialize( val.Url );
         }

      static IndexerIngest FromWire( StringRef buf, size_t *consumed )
         {
         size_t sizeWords;
         auto words = Deserialize<Vector<String>>( buf, &sizeWords );
         buf = buf.Substr( sizeWords );

         size_t sizeTitleWords;
         auto titleWords = Deserialize<Vector<String>>( buf, &sizeTitleWords );
         buf = buf.Substr( sizeTitleWords );

         size_t sizeUrl;
         auto url = Deserialize<String>( buf, &sizeUrl );

         *consumed = sizeWords + sizeTitleWords + sizeUrl;

         return IndexerIngest{ .Words = words,
                               .TitleWords = titleWords,
                               .Url = url };
         }
   };

#endif // SCOOBLE_SEARCH_INDEXERINGEST_H
