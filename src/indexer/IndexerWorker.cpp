//
// Created by youh on 3/14/21.
//

#include "../stl/utils.h"
#include "IndexChunk.h"
#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <pthread.h>

void *threadPersistChunk( void *p )
   {
   // get current time
   time_t timestamp = std::time( nullptr );
   tm *timeRepr = std::gmtime( &timestamp );
   char timeStr[ 1000 ];
   strftime( timeStr, 1000, "%Y_%m_%d_%H_%M_%S_", timeRepr );
   String filename( timeStr );
   filename += RandAlphaString( 6 ) + ".chunk";

   auto *chunk = static_cast<IndexChunk *>( p );
   chunk->writeToDisk( filename.data() );

   std::cout << "written chunk to " << filename.cstr() << std::endl;
   delete chunk;
   return nullptr;
   }

scb::String RandAlphaString( int len )
   {
   scb::String str;
   for ( int i = 0;  i < len;  i++ )
      str.pushBack( 'a' + rand() % 26 );
   return str;
   }

int main()
   {
   mqd_t mq = mq_open( PARSED_DOCS_QUEUE, O_RDONLY );
   auto *curChunk = new IndexChunk();

   while ( true )
      {
      char buf[ PARSED_DOCS_QUEUE_MAX_MSGSIZE ];
      ssize_t sz =
         mq_receive( mq, buf, PARSED_DOCS_QUEUE_MAX_MSGSIZE, nullptr );

      if ( sz > 0 )
         {
         String msg( buf, sz );
         size_t consumed;
         auto ingest =
            Deserialize<IndexerIngest>( StringRef( msg ), &consumed );
         assert( consumed == sz );

         curChunk->Ingest( ingest );
         if ( curChunk->EstSize >= INDEX_CHUNK_TARGET_SIZE )
            {
            auto *oldChunk = curChunk;
            curChunk = new IndexChunk();

            // spin up new thread to persist the old chunk
            pthread_t pth;
            pthread_create( &pth, nullptr, threadPersistChunk, oldChunk );
            pthread_detach( pth );
            }
         }
      }
   }