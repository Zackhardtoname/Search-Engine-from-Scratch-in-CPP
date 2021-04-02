//
// Created by youh on 3/14/21.
//

#include "../../indexer/IndexChunk.h"
#include "../../stl/utils.h"
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

scb::Vector<scb::String> words;

scb::String RandAlphaString( int len )
{
   scb::String str;
   for ( int i = 0;  i < len;  i++ )
      str.pushBack( 'a' + rand() % 26 );
   return str;
}

scb::String RandWord() {
   return words[rand() % words.size()];
}

scb::String RandURl() {
   scb::String res;

   // Protocol
   if( rand() % 3 == 0 ) {
      res += "http";
   }
   else {
      res += "https";
   }
   res += "://";

   // Subdomain
   if( rand() % 3 == 0 ) {
      res += RandWord();
      res += ".";
   }

   // Domain
   res += RandWord();
   res += ".";

   // Top-Level Domain
   switch(rand() % 4) {
      case 0:
         res += "edu";
         break;
      case 1:
         res += "org";
         break;
      default:
         res += "com";
         break;
   }

   // Path
   size_t numLevels = rand() % 4;
   for(size_t i = 0; i < numLevels ; ++i) {
      res += "/";
      res += RandWord();
   }

   res += "/";

   return res;
}

int main()
   {
   // Read in common words
   
   std::ifstream commonWordsStream;
   commonWordsStream.open ("commonWords.txt");

   std::string word;

   while(commonWordsStream >> word) {
      words.pushBack(String(word.c_str()));
   }

   mqd_t mq = 0;// mq_open( PARSED_DOCS_QUEUE, O_RDONLY );
   srand(time(0));

   auto *curChunk = new IndexChunk();

   scb::Vector<IndexerIngest> ingests; 

   // Create 2 - 4 ingests
   size_t numIngests = 2 + (rand() % 3);
   for ( size_t i = 0 ; i < numIngests ; ++i ) {
      
      IndexerIngest testIngest;

      // Generate 10-30 words
      size_t numWords = 10 + ( rand() % 21 );
      for(size_t j = 0; j < numWords; ++j) {
         testIngest.Words.pushBack(RandWord());
      }

      // Generate 2-5 title words
      numWords = 2 + ( rand() % 4 );
      for(size_t j = 0; j < numWords; ++j) {
         testIngest.TitleWords.pushBack(RandWord());
      }

      // Generate URL
      std::cout << RandURl().cstr() << std::endl;

      // Create Ingest, Print ingestfile
      auto msg = Serialize(testIngest);
      size_t consumed;
      std::ofstream ingestfile;
      std::string filename = "most_recent_" + std::to_string(i) + ".ingest";
      ingestfile.open (filename, std::ios::trunc);
      ingestfile << msg.data();
      ingestfile.close();
      auto ingest = Deserialize<IndexerIngest>( StringRef( msg ), &consumed );
      assert( consumed == msg.size() );
      curChunk->Ingest( ingest );

   }

   threadPersistChunk(curChunk);
}