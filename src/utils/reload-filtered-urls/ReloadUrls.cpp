#include "../../defs.h"
#include "../../stl/Filesystem.h"
#include "../../stl/RAII.h"
#include "../../stl/utils.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <sys/wait.h>

void reload()
   {

   mqd_t frontier = mq_open( FRONTIER_MQ, O_WRONLY );
   if ( frontier == -1 )
      {
      std::cout << "mq_open() failed. Make sure you created the frontier queue "
                   "in the src/utils/handle-mqs folder with:"
                << std::endl;
      std::cout << "  make" << std::endl;
      std::cout << "  ./handle-mq frontier newLinks" << std::endl;
      return;
      }

   scb::String url;
   while ( std::cin >> url )
      {

      // Only send up to the length of the message to avoid sending too much
      // data to the mqueue
      if ( url.length() > MAX_URL_LEN )
         continue;

      if ( mq_send( frontier, url.data(), url.length(), 0 ) == -1 )
         {
         // Check if the frontier is full before adding another URL
         if ( errno == EAGAIN )
            {
            mq_attr attr{};
            mq_getattr( frontier, &attr );
            std::cout << "Message queue has reached the max capacity of "
                      << attr.mq_maxmsg << std::endl;
            break;
            }
         else
            assert( false && "mq_send() error" );
         }
      else
         std::cout << "Queued message: " << url << std::endl;
      }

   // cleanup
   assert( (mqd_t)-1 != mq_close( frontier ) );
   }

int main( int argc, char *argv[] )
   {
   reload();
   return 0;
   }
