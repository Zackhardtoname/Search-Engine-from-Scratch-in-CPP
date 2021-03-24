//
// Created by youh on 3/14/21.
//

#include "IndexChunk.h"
#include <cassert>
#include <mqueue.h>

int main()
   {
   mqd_t mq = mq_open( PARSED_DOCS_QUEUE, O_RDONLY );

   auto *curChunk = new IndexChunk();

   while ( true )
      {
      char buf[ PARSED_DOCS_QUEUE_MAX_MSGSIZE ];
      ssize_t sz =
         mq_receive( mq, buf, PARSED_DOCS_QUEUE_MAX_MSGSIZE, nullptr );

      String msg( buf, sz );
      size_t consumed;
      auto ingest = Deserialize<IndexerIngest>( StringRef( msg ), &consumed );
      assert( consumed == sz );

      curChunk->Ingest( ingest );
      if ( curChunk->EstSize >= INDEX_CHUNK_TARGET_SIZE )
         {
         auto *oldChunk = curChunk;
         // TODO: spin up new thread to persist the old chunk
         curChunk = new IndexChunk();
         }
      }
   }