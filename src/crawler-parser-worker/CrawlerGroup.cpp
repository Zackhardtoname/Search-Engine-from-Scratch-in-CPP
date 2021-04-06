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

void SIGINTTERMHandler( int )
   {
   die.store( true );
   }

int main( int argc, char **argv )
   {
   if ( argc != 3 || scb::String( argv[ 1 ] ) == "-h" ||
        scb::String( argv[ 1 ] ) == "--help" )
      {
      std::cerr << "Usage: " << argv[ 0 ]
                << " [num-crawler] [stemmed-english-dict]\n";
      std::cerr << "This crawler group controller starts a number of crawler "
                   "workers and makes sure they are alive\n";

      return 1;
      }

   char *endptr;
   int numChildren = strtol( argv[ 1 ], &endptr, 10 );
   if ( *endptr )
      {
      std::cerr << "cannot parse number of children " << argv[ 1 ] << std::endl;
      return 1;
      }

   // These mqueues and the englishDictChecker is opened once by this
   // group, and inherited by the children workers.
   // If we want to go with multiple queues with different priorities,
   //    add flag O_NONBLOCK and poll from them.
   frontier = mq_open( FRONTIER_MQ, O_RDONLY );
   if ( frontier == -1 )
      {
      std::cerr << "mq_open() failed for frontier: " << strerror( errno )
                << std::endl;
      return 1;
      }

   newLinks = mq_open( NEW_LINKS_QUEUE, O_WRONLY );
   if ( newLinks == -1 )
      {
      std::cerr << "mq_open() failed for newLinks: " << strerror( errno )
                << std::endl;
      return 1;
      }

   parsedDocs = mq_open( PARSED_DOCS_QUEUE, O_WRONLY );
   if ( parsedDocs == -1 )
      {
      std::cerr << "mq_open() failed for parsedDocs: " << strerror( errno )
                << std::endl;
      return 1;
      }

   // get resources for the children
   englishDictChecker = new EnglishDictChecker( argv[ 2 ] );

   // Register signals and signal handlers
   struct sigaction SIGINTTERMAction
      {
      };
   SIGINTTERMAction.sa_handler = SIGINTTERMHandler;
   SIGINTTERMAction.sa_flags =
      0; // no SA_RESTART so that wait() can be interrupted
   sigaction( SIGTERM, &SIGINTTERMAction, nullptr );
   sigaction( SIGINT, &SIGINTTERMAction, nullptr );

   std::cout << "crawler controller starting" << std::endl;
   // spin-up NUM_CRAWLERS crawlers
   for ( int i = 0;  i < numChildren;  ++i )
      {
      pthread_t child;
      pthread_create( &child, nullptr, ThreadCrawlerParserWorker, nullptr );
      pthread_detach( child );
      }

   pthread_exit( nullptr );
   }
