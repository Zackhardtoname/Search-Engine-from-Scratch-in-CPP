//#include <assert.h>
#include <cassert>
//#include <cstdint>
//#include <cstdlib>
//#include <cstring>
#include <iostream>
#include <mqueue.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../defs.h"
#include "../../stl/Filesystem.h"
#include "../../stl/RAII.h"
#include "../../stl/utils.h"

void seed( int argc )
   {

   mqd_t frontier = mq_open( FRONTIER_MQ, O_WRONLY );
   assert( frontier != -1 && "mq_open() failed" );

   char FileNameBuff[ FILENAME_MAX ];
   //getcwd( FileNameBuff, FILENAME_MAX );
   scb::String cwd( FileNameBuff );
   scb::String seedPath = cwd + "/data/us_top_100.txt";
   const char *seedPathCStr = seedPath.cstr();
   char *seedStart;
   char *seedEnd;
   mapfile( &seedPathCStr, &seedStart, &seedEnd );
   char *seedFilePtr = seedStart;

   int curCrawlerNum = 0;

   while ( seedFilePtr != seedEnd )
      {
      char URLBuff[ MAX_URL_LEN ];
      char *URLBuffPtr = URLBuff;

      while ( *seedFilePtr != '\n' )
         {
         *URLBuffPtr = *seedFilePtr;
         URLBuffPtr++;
         seedFilePtr++;
         }
      *URLBuffPtr = '\0';

      scb::String message = scb::String( URLBuff );
      // Leaving this if statement out since this is handled in
      // CrawlerParserWorker.cpp (for now)
      // if (message.substr(0, 4) != "http") {
      //   message = "http://www." + message + "\0";
      //}

      assert( 0 <= mq_send( frontier, message.cstr(), MAX_URL_LEN, 0 ) );
      std::cout << "Queued message: " << message.cstr() << std::endl;

      seedFilePtr++;
      }

   // cleanup
   assert( (mqd_t)-1 != mq_close( frontier ) );
   }

int main( int argc, char *argv[] )
   {
   seed( argc );
   return 0;
   }
