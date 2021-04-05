//
// Created by youh on 3/21/21.
//

#include "FilterWorker.h"
#include "../crawler/Crawler.h"
#include "../stl/Counter.h"
#include "../stl/RAII.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

using scb::String;
using scb::Vector;

Robotstxt *const FilterWorker::ROBOTS_TXT_PENDING =
   reinterpret_cast<Robotstxt *const>( 1 );
Robotstxt *const FilterWorker::DummyRobotstxt =
   new Robotstxt( "User-agent: *\n" );

void FilterWorker::Run()
   {
   // mask SIGINT such that our children have that masked by default
   sigset_t sigset;
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGINT );
   pthread_sigmask( SIG_BLOCK, &sigset, nullptr );

   // spawn the threads
   pthread_create( &pthNewLinksMqueue, nullptr, threadNewLinksMqueue, this );
   pthread_create( &pthRemoteWorkers, nullptr, threadRemoteWorkers, this );

   for ( pthread_t &pth : pthUnfilteredUrls )
      pthread_create( &pth, nullptr, threadUnfilteredUrls, this );

   pthread_create( &pthRemoteUrls, nullptr, threadRemoteUrls, this );
   pthread_create( &pthFilteredUrls, nullptr, threadFilteredUrls, this );

   // wait on any one of the threads
   pthread_join( pthNewLinksMqueue, nullptr );
   }

bool FilterWorker::markIfNotSeen( const String &url )
   {
   return bloomfilter.Insert( url );
   }

int FilterWorker::getHostRateLimit( const String &host )
   {
   return 1; // TODO: read alexa
   }

bool FilterWorker::allowedByRobotstxt( const ParsedUrl &url )
   {
   pthread_mutex_lock( &lockRobotstxtMap );

   // if no robots.txt yet, fetch and set
   if ( !robotstxtMap.count( url.Host ) )
      {
      std::cout << "no robots.txt for " << url.Host << std::endl;

      // we'll take the responsibility of downloading it, set as pending
      robotstxtMap[ url.Host ] = ROBOTS_TXT_PENDING;
      auto *cond = new pthread_cond_t;
      pthread_cond_init( cond, nullptr );

      // unlock to allow other threads to proceed
      pthread_mutex_unlock( &lockRobotstxtMap );

      // if either downloading failed, or parsing failed, use dummy robots.txt
      try
         {
         auto result = FetchUrl( url.Normalize( "robots.txt" ) );
         auto *robotstxt = new Robotstxt( result );
         pthread_mutex_lock( &lockRobotstxtMap );
         robotstxtMap[ url.Host ] = robotstxt;
         std::cout << "robots.txt for " << url.Host << " downloaded and parsed"
                   << std::endl;
         }
      catch ( std::exception &ex )
         {
         pthread_mutex_lock( &lockRobotstxtMap );
         robotstxtMap[ url.Host ] = DummyRobotstxt;
         std::cout << "robots.txt for " << url.Host << " failed: " << ex.what()
                   << std::endl;
         }

      // signal other threads
      pthread_cond_broadcast( &pendingRobotstxtCond );
      }

   // now that we can be sure that we have a robots.txt,
   // or at least a pending one
   Robotstxt *robotstxt = nullptr;

   // while pending, sleep until it is ready
   while ( ( robotstxt = robotstxtMap[ url.Host ] ) == ROBOTS_TXT_PENDING )
      {
      std::cout << "waiting for robots.txt of " << url.Host << std::endl;
      pthread_cond_wait( &pendingRobotstxtCond, &lockRobotstxtMap );
      }

   // check if allowed
   bool allowed = robotstxt->Allow( url.CompleteUrl );
   pthread_mutex_unlock( &lockRobotstxtMap );
   return allowed;
   }

FilterWorker::FilterWorker( const char *remoteConfigFile, int rk,
                            const char *bloomFile )
    : bloomfilter( bloomFile )
   {
   // open the queues
   qNewLinks = mq_open( NEW_LINKS_QUEUE, O_RDONLY );
   if ( qNewLinks == -1 )
      {
      std::cerr << "mq_open(NEW_LINKS_QUEUE) failed: " << strerror( errno )
                << std::endl;
      exit( 1 );
      }

   qFrontier = mq_open( FRONTIER_MQ, O_WRONLY | O_NONBLOCK );
   if ( qFrontier == -1 )
      {
      std::cerr << "mq_open(FRONTIER_MQ) failed: " << strerror( errno )
                << std::endl;
      exit( 1 );
      }

   // initialize sync structures
   assert( sem_init( &semUnfilteredUrls, 0, 0 ) != -1 && "sem_init() failed" );
   assert( pthread_mutex_init( &lockRemoteWorkersSendBuffer, nullptr ) == 0 &&
           "mutex_init() failed" );
   assert( pthread_mutex_init( &lockFilteredUrls, nullptr ) == 0 &&
           "mutex_init() failed" );
   assert( pthread_mutex_init( &lockUnfilteredUrls, nullptr ) == 0 &&
           "mutex_init() failed" );
   assert( pthread_mutex_init( &lockRobotstxtMap, nullptr ) == 0 &&
           "mutex_init() failed" );

   assert( pthread_cond_init( &pendingRobotstxtCond, nullptr ) == 0 &&
           "cond_init() failed" );

   // setup remote
   this->rank = rk;
   std::ifstream remoteConf( remoteConfigFile );
   if ( !remoteConf )
      assert( !"cannot open() remote config file" );
   remoteConf >> worldSize;
   assert( worldSize > 0 && "world size must be positive" );
   assert( worldSize < 1000 && "world size sanity check" );
   assert( 0 <= rk && rk < worldSize && "invalid rank" );
   std::cout << "I'm rank " << rk << " in a world size " << worldSize
             << std::endl;
   remoteWorkersSendBuffer.resize( worldSize );
   for ( int i = 0;  i < worldSize;  i++ )
      {
      scb::String addr;
      remoteConf >> addr;
      ParsedUrl parsed( addr );
      remoteWorkerAddrs.pushBack( parsed );
      std::cout << "remote " << i << "/" << ( worldSize - 1 ) << " is "
                << parsed.Host << ':' << parsed.Port << std::endl;
      if ( i == rk )
         {
         hostname = parsed.Host;
         port = parsed.Port;
         }
      }
   }

void FilterWorker::block()
   {
   pthread_mutex_lock( &lockUnfilteredUrls );
   pthread_mutex_lock( &lockRemoteWorkersSendBuffer );
   pthread_mutex_lock( &lockFilteredUrls );
   pthread_mutex_lock( &lockRobotstxtMap );
   }

void FilterWorker::unblock()
   {
   pthread_mutex_unlock( &lockUnfilteredUrls );
   pthread_mutex_unlock( &lockRemoteWorkersSendBuffer );
   pthread_mutex_unlock( &lockFilteredUrls );
   pthread_mutex_unlock( &lockRobotstxtMap );
   }

int main( int argc, char **argv )
   {

   if ( argc != 4 )
      {
      fprintf( stderr, "Usage: %s [remote-config] [bloom-file] [rank]\n",
               argv[ 0 ] );
      return 1;
      }

   char *endptr;
   int rank = strtol( argv[ 3 ], &endptr, 10 );
   if ( *endptr )
      {
      fprintf( stderr, "cannot parse rank %s\n", argv[ 3 ] );
      return 1;
      }

   FilterWorker worker( argv[ 1 ], rank, argv[ 2 ] );
   worker.Run();
   }