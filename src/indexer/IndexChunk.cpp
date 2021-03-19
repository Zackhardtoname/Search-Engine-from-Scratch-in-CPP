//
// Created by youh on 3/14/21.
//

#include "IndexChunk.h"

void IndexChunk::Ingest( IndexerIngest &injest )
   {
   for ( String &word : injest.Words )
      postingLists[ word ].pushBack( nextId++ );
   for ( String &word : injest.TitleWords )
      postingLists[ String( "@" ) + word ].pushBack( nextId++ );
   documents.pushBack( { .Url = injest.Url, .Boundary = nextId } );
   // TODO: update estimated size
   }

void IndexChunk::Persist()
   {
   // TODO: make this a mmap-ed file
   // Structure:
   // [Header] [Data]
   //
   // Header:
   // [i32 num of documents] [i32 offset for doc boundary list] [i32 offset for
   // URL offset list] [i32 offset for URL list] [i32 offset for posting list
   // idx] [i32 offset for posting lists]
   //
   // Data:
   // [doc boundary list] [URL offset list] [URL list] [posting list idx]
   // [posting lists]
   //
   // Document boundary list:
   // num_doc entries
   // Each entry: [i32 boundary] past-one boundary of this document
   //
   // URL offset list:
   // num_doc entries
   // Each entry: [i32 offset]
   }
