/*
   The Seeder is supposed to run once to seed the URL queues with some seedlist.
   The seedlist should be passed in via standard input (using pipe on the command line)
*/

#include "../../defs.h"
#include "../../stl/Filesystem.h"
#include "../../stl/RAII.h"
#include "../../stl/utils.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <sys/wait.h>

void seed()
   {

   mqd_t newLinks = mq_open( NEW_LINKS_QUEUE, O_WRONLY | O_NONBLOCK );
   if ( newLinks == -1 )
      {
      std::cout << "mq_open() failed. Make sure you created the newLinks queue "
                   "in the src/utils/handle-mqs folder with:"
                << std::endl;
      std::cout << "  make" << std::endl;
      std::cout << "  ./handle-mq create newLinks" << std::endl;
      return;
      }

   scb::String url;
   while ( std::cin >> url )
      {

      // Only send up to the length of the message to avoid sending too much
      // data to the mqueue
      if ( url.length() > MAX_URL_LEN )
         // TODO ASK this seems a bit strange - cin will be consumed if we just continue; not sure if we mean "break"
         continue;

      if ( mq_send( newLinks, url.data(), url.length(), 0 ) == -1 )
         {
         // Check if the frontier is full before adding another URL
         if ( errno == EAGAIN )
            {
            mq_attr attr{};
            mq_getattr( newLinks, &attr );
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
   assert( (mqd_t)-1 != mq_close( newLinks ) );
   }

int main( int argc, char *argv[] )
   {
   seed();
   return 0;
   }
