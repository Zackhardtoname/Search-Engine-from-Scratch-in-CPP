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

   mq_attr attr{};
   mqd_t frontier = mq_open( FRONTIER_MQ, O_WRONLY | O_NONBLOCK, 0644, &attr );
   if ( frontier == -1 )
      {
      std::cout << "mq_open() failed. Make sure you created the frontier queue in the src/utils/handle-mqs folder with:" << std::endl;
      std::cout << "  make" << std::endl;
      std::cout << "  ./handle-mq create frontier" << std::endl;
      return;
      }

   char FileNameBuff[ PATH_MAX ];
   getcwd( FileNameBuff, PATH_MAX );
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
      if ( mq_send( frontier, message.cstr(), message.length(), 0 ) == -1 )
         {
         // Check if the frontier is full before adding another URL
         if ( errno == EAGAIN )
            {
            mq_getattr( frontier, &attr);
            std::cout << "Message queue has reached the max capacity of "
                      << attr.mq_maxmsg << std::endl;
            break;
            }
         else
            assert( false && "mq_send() error" );
         }
      else
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
