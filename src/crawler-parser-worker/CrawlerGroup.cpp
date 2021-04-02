#include "../defs.h"
#include "../stl/RAII.h"
#include "../stl/utils.h"
#include "CrawlerParserWorker.h"
#include "EnglishDictChecker.h"
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <sys/wait.h>
#include <unistd.h>

bool interruptReceived = false;

void SIGINTHandler( int )
   {
   interruptReceived = true;
   }

void SIGTERMHandler( int )
   {
   // so that we do not get killed by killpg() and can die after reaping our
   // children.
   }

int main( int argc, char **argv )
   {
   if ( argc != 3 || scb::String( argv[ 1 ] ) == "-h" ||
        scb::String( argv[ 1 ] ) == "--help" )
      {
      std::cerr << "Usage: " << argv[ 0 ]
                << " [group-size] [stemmed-english-dict]\n";
      std::cerr << "This crawler group controller starts a number of crawler "
                   "workers and makes "
                   "sure they are alive\n";

      return 1;
      }

   char *endptr;
   int numChildren = strtol( argv[ 1 ], &endptr, 10 );
   if ( *endptr )
      {
      std::cerr << "cannot parse number of children " << argv[ 1 ] << std::endl;
      return 1;
      }

   // If we want to go with multiple queues with different priorities,
   //    add flag O_NONBLOCK and poll from them.
   mqd_t frontier = mq_open( FRONTIER_MQ, O_RDONLY );
   assert( frontier != -1 && "mq_open() failed" );

   mqd_t newLinks = mq_open( NEW_LINKS_QUEUE, O_WRONLY );
   assert( newLinks != -1 && "mq_open() failed" );

   mqd_t parsedDocs = mq_open( PARSED_DOCS_QUEUE, O_WRONLY );
   assert( parsedDocs != -1 && "mq_open() failed" );

   // get resources for the children
   EnglishDictChecker englishDictChecker( argv[ 2 ] );

   // Register signals and signal handlers
   // our children would register their own handler
   signal( SIGINT, SIGINTHandler );
   signal( SIGTERM, SIGTERMHandler );

   std::cout << "crawler controller starting" << std::endl;
   // spin-up NUM_CRAWLERS crawlers
   for ( int i = 0;  i < numChildren;  ++i )
      if ( pid_t child = fork() )
         {
         std::cout << "started child " << child << std::endl;
         }
      else
         return CrawlerParserWorkerMain( frontier, newLinks, parsedDocs,
                                         englishDictChecker );

   // watch for death of children and restart them
   int childStatus;
   while ( pid_t deadChild = wait( &childStatus ) )
      {
      if ( deadChild == -1 )
         {
         std::cout << "wait(): " << strerror( errno ) << std::endl;
         return 255;
         }

      if ( interruptReceived )
         {
         std::cout << "interrupt received... killing all children" << std::endl;
         killpg( 0, SIGTERM );
         while ( wait( nullptr ) != -1 )
            ;
         return 0;
         }

      std::cout << "child pid = " << deadChild
                << " exited status = " << WEXITSTATUS( childStatus )
                << std::endl;
      if ( pid_t child = fork() )
         {
         sleep(
            1 ); // this gives us chance to abort if children exit immediately
         std::cout << "started child " << child << std::endl;
         }
      else
         return CrawlerParserWorkerMain( frontier, newLinks, parsedDocs,
                                         englishDictChecker );
      }

   return 0;
   }
