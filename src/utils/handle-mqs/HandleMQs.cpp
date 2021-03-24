//
// Created by epfeiffe on 3/18/21.
//

#include "../../defs.h"
#include "../../stl/String.h"
#include <cassert>
#include <iostream>
#include <mqueue.h>

void handleMqueue( int argc, char **argv, scb::String operation )
   {
   for ( int i = 2;  i < argc;  ++i )
      {
      mqd_t mqueue;
      const char *mqueueName;
      // Delete the message queue listed
      if ( scb::String( argv[ i ] ) == "frontier" )
         mqueueName = FRONTIER_MQ;
      else if ( scb::String( argv[ i ] ) == "newLinks" )
         mqueueName = NEW_LINKS_QUEUE;
      else
         {
         std::cout << argv[ i ] << " is not a valid mqueue. Shutting down."
                   << std::endl;
         return;
         }
      // TODO: Add more else ifs here as more mqueues are added

      if ( operation == "create" )
         {

         if ( mq_open( mqueueName, O_RDONLY ) != -1 )
            {
            std::cout << argv[ i ] << " mqueue already exists, skipping for now"
                      << std::endl;
            continue;
            }

         mq_attr attr{};
         // initialize the queue attributes
         // TODO: Figure out how to set attr.mq_maxmsg to a higher value
         // TODO: Consider tweaking these attributes for each individual queue
         // type For example, we may want the frontier to have a size of 1000
         // but newLinks to only have a size of 500
         attr.mq_flags = 0;
         attr.mq_maxmsg = MAX_QUEUE_SIZE;
         attr.mq_msgsize = MAX_URL_LEN;
         attr.mq_curmsgs = 0;

         mqueue = mq_open( mqueueName, O_CREAT | O_RDONLY, 0644, &attr );
         assert( mqueue != -1 && "mq_open() failed" );
         std::cout << argv[ i ] << " mqueue successfully created!" << std::endl;
         }
      else if ( operation == "check" )
         {

         mq_attr attr{};
         mqueue = mq_open( mqueueName, O_RDONLY, 0644, &attr );
         if ( mqueue == -1 )
            {
            std::cout << "mq_open() failed for " << argv[ i ]
                      << " since it may not have been created" << std::endl;
            continue;
            }

         mq_getattr( mqueue, &attr );
         std::cout << argv[ i ] << " mqueue has " << attr.mq_curmsgs << "/"
                   << attr.mq_maxmsg << " elements" << std::endl;
         }
      else if ( operation == "delete" )
         {

         mqueue = mq_open( mqueueName, O_RDONLY | O_NONBLOCK );
         if ( mqueue == -1 )
            {
            std::cout << "mq_open() failed for " << argv[ i ]
                      << " since it may not have been created" << std::endl;
            continue;
            }
         // TODO: Use a longer MAX_URL_LEN for the parsed index mqueue
         mq_attr attr{};
         mq_getattr( mqueue, &attr );
         char *buf = new char[ attr.mq_msgsize ];
         int sz;
         while ( ( sz = mq_receive( mqueue, buf, attr.mq_msgsize, nullptr ) ) !=
                 -1 )
            {
            std::cout << "removed from " << argv[ i ] << ":  ";
            for ( int j = 0;  j < sz;  j++ )
               std::cout << buf[ j ];
            std::cout << std::endl;
            }
         delete[] buf;

         // Remove the mqueue from the system
         mq_unlink( mqueueName );
         std::cout << argv[ i ] << " mqueue successfully deleted!" << std::endl;
         }
      else
         {
         std::cout << operation.cstr()
                   << " is not a valid operation. Shutting down." << std::endl;
         exit( 1 );
         }
      }
   }

int main( int argc, char **argv )
   {
   if ( argc < 2 || scb::String( argv[ 1 ] ) == "-h" ||
        scb::String( argv[ 1 ] ) == "--help" )
      {
      std::cout << "Usage: ./handle-mq [mqueue-operation] [mqueue-name(s)]"
                << std::endl;
      std::cout << "Where [mqueue-operation] is exactly one of the following "
                   "operations:"
                << std::endl;
      std::cout << "  create" << std::endl;
      std::cout << "  check" << std::endl;
      std::cout << "  delete" << std::endl;
      std::cout << "And where [mqueue-name(s)] can be one or more of the "
                   "following queues seperated by a space:"
                << std::endl;
      std::cout << "  frontier" << std::endl;
      std::cout << "  newLinks" << std::endl;
      // TODO: Add more mqueues here as necessary, such as the index mqueue
      return 0;
      }

   scb::String operation = scb::String( argv[ 1 ] );

   handleMqueue( argc, argv, operation );
   }
