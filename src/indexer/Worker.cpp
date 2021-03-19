//
// Created by youh on 3/14/21.
//

#include "IndexChunk.h"

int main()
   {
   auto *curChunk = new IndexChunk();
   while ( true )
      {
      IndexerIngest ingest = read_data_from_Q3();
      curChunk->Ingest( ingest );
      if ( curChunk->estSize >= INDEX_CHUNK_TARGET_SIZE )
         {
         auto *oldChunk = curChunk;
         // TODO: spin up new thread to persist the old chunk
         curChunk = new IndexChunk();
         }
      }
   }