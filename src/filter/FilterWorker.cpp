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

const Robotstxt FilterWorker::DummyRobotstxt( "User-agent: *\n" );

void FilterWorker::Run()
   {
   // mask SIGINT such that our children have that masked by default
   sigset_t sigset;
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGINT );
   pthread_sigmask( SIG_BLOCK, &sigset, nullptr );

   assert( curl_global_init( CURL_GLOBAL_DEFAULT ) == 0 );

   // spawn the threads
   for ( pthread_t &pth : pthNewLinksMqueue )
      pthread_create( &pth, nullptr, threadNewLinksMqueue, this );
   pthread_create( &pthRemoteWorkers, nullptr, threadRemoteWorkers, this );

   pthread_create( &pthRemoteUrls, nullptr, threadRemoteUrls, this );
   pthread_create( &pthFilteredUrls, nullptr, threadFilteredUrls, this );

   pthread_create( &pthMonitor, nullptr, threadMonitor, this );

   pthread_create( &pthOffloadUnfilteredUrls, nullptr,
                   threadOffloadUnfilteredUrls, this );

   // wait on any one of the threads
   pthread_join( pthNewLinksMqueue[ 0 ], nullptr );
   }

bool FilterWorker::markIfNotSeen( const String &url )
   {
   return bloomfilter.Insert( url );
   }

int FilterWorker::getHostRateLimit( const String &host )
   {
   return 1; // TODO: read alexa
   }

void *FilterWorker::threadFetchRobotstxt( void *p )
   {
   scb::UniquePointer args( static_cast<FetchRobotstxtArgs *>( p ) );

   // if either downloading failed, or parsing failed, use dummy robots.txt
   String content;
   CURL *curl = GetCurlHandle( &content );
   try
      {
      auto robotstxtUrl = args->Url.Normalize( "robots.txt" );
      curl_easy_setopt( curl, CURLOPT_URL, robotstxtUrl.cstr() );
      CURLcode code = curl_easy_perform( curl );
      HandleCurlCode( curl, code );

      Robotstxt robotstxt( content ); // warning: this may throw
      curl_easy_cleanup( curl );
      pthread_mutex_lock( &args->self->lockRobotstxtMap );
      args->self->robotstxtLRU.emplace_front( args->Url.Host,
                                              std::move( robotstxt ) );
#ifdef VERY_VERBOSE
      std::cout << "robots.txt for " << args->Url.Host
                << " downloaded and parsed" << std::endl;
#endif
      }
   catch ( std::exception &ex )
      {
      curl_easy_cleanup( curl );
      pthread_mutex_lock( &args->self->lockRobotstxtMap );
      args->self->robotstxtLRU.emplace_front( args->Url.Host, DummyRobotstxt );
      std::cerr << "robots.txt " << args->Url.Normalize( "robots.txt" )
                << " failed: " << ex.what() << std::endl;
      }

   // here, robotstxtLRU.front() must be a robots.txt, albeit maybe dummy
   CacheInfo &cacheInfo = args->self->robotstxtMap[ args->Url.Host ];
   cacheInfo.Pointer = args->self->robotstxtLRU.begin();
   cacheInfo.Pending = false;

   // take over all pending URLs
   Vector<ParsedUrl> unfiltered = std::move( cacheInfo.WaitingURLs );
   cacheInfo.WaitingURLs.clear();
   // take a copy of robotstxt so we can release lock ASAP
   Robotstxt robotstxt = args->self->robotstxtLRU.front().robotstxt;

   // LRU evict
   if ( args->self->robotstxtLRU.size() > MAX_NUM_ROBOTSTXT )
      {
      args->self->robotstxtMap.erase( args->self->robotstxtLRU.back().Host );
      args->self->robotstxtLRU.pop_back();
      }

   pthread_mutex_unlock( &args->self->lockRobotstxtMap );

   // filter all links that we took over
   Vector<ParsedUrl> filtered;
   for ( ParsedUrl &pendingUrl : cacheInfo.WaitingURLs )
      if ( robotstxt.Allow( pendingUrl.CompleteUrl ) )
         {
         // filter by bloom filter
         if ( args->self->markIfNotSeen( pendingUrl.CompleteUrl ) )
            {
#ifdef VERY_VERBOSE
            std::cout << "seen: " << pendingUrl.CompleteUrl << std::endl;
#endif
            continue;
            }

         // collect all filtered URLs and later push them all to filtered mq
         filtered.pushBack( std::move( pendingUrl ) );
         }

   // all good! we can put them into filteredUrls
   pthread_mutex_lock( &args->self->lockFilteredUrls );
   for ( ParsedUrl &url : filtered )
      {
      args->self->filteredUrls.pushBack( std::move( url ) );
      ++args->self->flowoutCount;
      }
   pthread_mutex_unlock( &args->self->lockFilteredUrls );

   return nullptr;
   }

void FilterWorker::handleUnfilteredLink( const String &link )
   {
   ++flowinCount;

   ParsedUrl url( link );

   // filter by domain hash
   int rightfulHandler = url.Host.hash() % worldSize;
   if ( rightfulHandler != rank )
      {
#ifdef VERY_VERBOSE
      std::cout << "not our host: " << url.Host << std::endl;
#endif
      queueRemoteUrl( rightfulHandler, url.CompleteUrl );
      return;
      }

   // filter by robots.txt
   pthread_mutex_lock( &lockRobotstxtMap );

   // if no robots.txt yet, fetch and set in a new thread
   if ( !robotstxtMap.count( url.Host ) )
      {
#ifdef VERY_VERBOSE
      std::cout << "no robots.txt for " << url.Host << std::endl;
#endif

      // we'll take the responsibility of downloading it, so set as pending
      robotstxtMap[ url.Host ] =
         CacheInfo{ .Pending = true, .WaitingURLs = { url } };

      // unlock to allow other threads to proceed
      pthread_mutex_unlock( &lockRobotstxtMap );

      pthread_t pth;
      auto *args = new FetchRobotstxtArgs{ this, url };
      pthread_create( &pth, nullptr, threadFetchRobotstxt, args );
      pthread_detach( pth );

      return;
      }

   // here we can be sure that we have a robots.txt,
   // or at least a pending one
   auto cache = robotstxtMap[ url.Host ];

   // if pending, add this url to the pending list and let the other thread
   // handle it
   if ( cache.Pending )
      {
#ifdef VERY_VERBOSE
      std::cout << "waiting for robots.txt of " << url.Host << std::endl;
#endif
      cache.WaitingURLs.emplaceBack( std::move( url ) );
      pthread_mutex_unlock( &lockRobotstxtMap );
      return;
      }

   // we have an immediately available robots.txt
   // check if allowed
   bool allowed = cache.Pointer->robotstxt.Allow( url.CompleteUrl );
   pthread_mutex_unlock( &lockRobotstxtMap );
   if ( allowed )
      {
      if ( markIfNotSeen( url.CompleteUrl ) )
         {
#ifdef VERY_VERBOSE
         std::cout << "seen: " << url.CompleteUrl << std::endl;
#endif
         return;
         }

      // all good! we can put that into filteredUrls
      pthread_mutex_lock( &lockFilteredUrls );
      filteredUrls.pushBack( std::move( url ) );
      ++flowoutCount;
      pthread_mutex_unlock( &lockFilteredUrls );
      }
   else
      {
#ifdef VERY_VERBOSE
      std::cout << "not allowed by robots.txt: " << url.CompleteUrl
                << std::endl;
#endif
      return;
      }
   }

FilterWorker::FilterWorker( const char *remoteConfigFile, int rk,
                            const char *bloomFile )
    : bloomfilter( bloomFile )
   {
   // open the queues
   qNewLinks = mq_open( NEW_LINKS_QUEUE, O_RDWR );
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

   assert( pthread_mutex_init( &lockRemoteWorkersSendBuffer, nullptr ) == 0 &&
           "mutex_init() failed" );
   assert( pthread_mutex_init( &lockFilteredUrls, nullptr ) == 0 &&
           "mutex_init() failed" );
   assert( pthread_mutex_init( &lockRobotstxtMap, nullptr ) == 0 &&
           "mutex_init() failed" );

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
   pthread_mutex_lock( &lockRemoteWorkersSendBuffer );
   pthread_mutex_lock( &lockFilteredUrls );
   pthread_mutex_lock( &lockRobotstxtMap );
   }

void FilterWorker::unblock()
   {
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