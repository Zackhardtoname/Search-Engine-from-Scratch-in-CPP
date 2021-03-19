#include "../defs.h"
#include "../stl/Filesystem.h"
#include "../stl/RAII.h"
#include "../stl/utils.h"
#include <assert.h>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mqueue.h>
#include <sys/wait.h>
#include <unistd.h>

void seed( int argc )
   {
   // TODO question - can the attr be shared so to avoid recreating a ton of
   // them...? PubClient producer;
   struct mq_attr attr;
   attr.mq_flags = 0; // blocking
   attr.mq_maxmsg = 256;
   // Maximal url size most browsers support
   attr.mq_msgsize = MAX_URL_LEN;
   attr.mq_curmsgs = 0;
   /* create the message queue */
   mqd_t frontier = mq_open( FRONTIER_MQ, O_CREAT | O_RDWR, 0644, &attr );
   assert( (mqd_t)-1 != frontier );

   char FileNameBuff[ FILENAME_MAX ];
   getcwd( FileNameBuff, FILENAME_MAX );
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
      std::cout << "Queued message: " << message << std::endl;

      seedFilePtr++;
      }

   // cleanup
   assert( (mqd_t)-1 != mq_close( frontier ) );
   assert( (mqd_t)-1 != mq_unlink( FRONTIER_MQ ) );
   }

int main( int argc, char *argv[] )
   {
   seed( argc );
   return 0;
   }
