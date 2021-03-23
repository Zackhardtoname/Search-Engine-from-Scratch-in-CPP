#include "../../defs.h"
#include "../../stl/Filesystem.h"
#include "../../stl/RAII.h"
#include "../../stl/utils.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <sys/wait.h>

void seed( int argc )
   {

   mqd_t frontier = mq_open( FRONTIER_MQ, O_WRONLY );
   assert( frontier != -1 && "mq_open() failed" );

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

      // Only send up to the length of the message to avoid sending too much
      // data to the mqueue
      assert( 0 <= mq_send( frontier, message.cstr(), message.length(), 0 ) );
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
