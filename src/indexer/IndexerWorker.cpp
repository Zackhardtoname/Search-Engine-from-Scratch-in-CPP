//
// Created by youh on 3/14/21.
//

#include "IndexChunk.h"
#include "../stl/utils.h"
#include <cassert>
#include <ctime>
#include <iostream>
#include <fstream>
#include <mqueue.h>
#include <pthread.h>


void *threadPersistChunk( void *p )
   {
   // get current time
   time_t timestamp = std::time( nullptr );
   tm *timeRepr = std::gmtime( &timestamp );
   char timeStr[ 1000 ];
   strftime(timeStr, 1000, "%Y_%m_%d_%H_%M_%S_", timeRepr );
   String filename(timeStr);
   filename += RandAlphaString(6) + ".chunk";

   auto *chunk = static_cast<IndexChunk *>( p );
   chunk->writeToDisk(filename.data() );

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
   mqd_t mq = 0;// mq_open( PARSED_DOCS_QUEUE, O_RDONLY );
   srand(time(0));

   auto *curChunk = new IndexChunk();

   // ===== Test =====

   IndexerIngest testIngest;

   testIngest.Words.pushBack("You");
   testIngest.Words.pushBack("say");
   testIngest.Words.pushBack("goodbye");
   testIngest.Words.pushBack("and");
   testIngest.Words.pushBack("I");
   testIngest.Words.pushBack("say");
   testIngest.Words.pushBack("hello");

   testIngest.TitleWords.pushBack("The");
   testIngest.TitleWords.pushBack("Title");

   testIngest.Url = "https://mi.chaelwolf.com";
   /*
   for (int i = 0; i < 10; i++)
      testIngest.Words.pushBack(RandAlphaString(10));
   for (int i = 0; i < 3; i++)
      testIngest.TitleWords.pushBack(RandAlphaString(10));
   testIngest.Url = RandAlphaString(50);
   */

   auto msg = Serialize(testIngest);
   size_t consumed;

   std::ofstream ingestfile;
   ingestfile.open ("most_recent.ingest", std::ios::trunc);
   ingestfile << msg.data();
   ingestfile.close();

   auto ingest = Deserialize<IndexerIngest>( StringRef( msg ), &consumed );
    assert( consumed == msg.size() );
   curChunk->Ingest( ingest );
   threadPersistChunk(curChunk);
   return 0;

   // ===== End Test =====

   while ( true )
      {
      char buf[ PARSED_DOCS_QUEUE_MAX_MSGSIZE ];
      ssize_t sz =
         mq_receive( mq, buf, PARSED_DOCS_QUEUE_MAX_MSGSIZE, nullptr );

      // ? can messages be split into multiple buffers?

      if( sz > 0 ) {
         String msg( buf, sz );
         size_t consumed;
         auto ingest = Deserialize<IndexerIngest>( StringRef( msg ), &consumed );

         // Pretty sure consumed doesn't necessarily equal size due to variable-length encoding
         // assert( consumed == sz );

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