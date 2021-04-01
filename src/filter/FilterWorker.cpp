//
// Created by youh on 3/21/21.
//

#include "FilterWorker.h"
#include "../crawler/Crawler.h"
#include "../defs.h"
#include "../indexer/IndexChunk.h"
#include "../stl/Counter.h"
#include "../stl/RAII.h"
#include "../stl/String.h"
#include "../stl/Vector.h"
#include "../stl/utils.h"
#include <cassert>
#include <csignal>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

using scb::String;
using scb::Vector;

void FilterWorker::run()
   {
   // spawn the threads
   pthread_create( &pthNewLinksMqueue, nullptr, threadNewLinksMqueue, this );
   pthread_create( &pthRemoteWorkers, nullptr, threadRemoteWorkers, this );

   for ( pthread_t &pth : pthUnfilteredUrls )
      pthread_create( &pth, nullptr, threadUnfilteredUrls, this );

   pthread_create( &pthRemoteUrls, nullptr, threadRemoteUrls, this );
   pthread_create( &pthFilteredUrls, nullptr, threadFilteredUrls, this );

   // wait on any one of the thraed
   pthread_join( pthNewLinksMqueue, nullptr );
   }

void *FilterWorker::threadNewLinksMqueue( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   char buf[ MAX_URL_LEN + 1 ];
   while ( true )
      {
      ssize_t sz = mq_receive( self.qNewLinks, buf, MAX_URL_LEN, nullptr );
      if ( sz == -1 )
         {
         std::cerr << "mq_receive() failed: " << strerror( errno ) << std::endl;
         exit( 1 );
         }

      buf[ sz ] = '\0';
      std::cout << "from local mq: " << buf << std::endl;

      pthread_mutex_lock( &self.lockUnfilteredUrls );
      self.unfilteredUrls.pushBack( String( buf ) );
      pthread_mutex_unlock( &self.lockUnfilteredUrls );

      // notify the downstream
      sem_post( &self.semUnfilteredUrls );
      }
   }

void *FilterWorker::threadRemoteWorkers( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   // create, bind, and listen socket
   addrinfo *addrResult;
   try
      {
      addrResult = QueryHost( self.hostname.cstr(), self.port.cstr() );
      }
   catch ( std::exception & )
      {
      assert( false && "fail to get the address to bind. cannot receive URLs "
                       "from remote workers." );
      }

   int sock = socket( addrResult->ai_family, addrResult->ai_socktype,
                      addrResult->ai_protocol );
   assert( sock != 0 && "error in socket()" );

   int yes = 1;
   int status =
      setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );
   assert( status == 0 && "error in setsockopt()" );

   status = bind( sock, addrResult->ai_addr, addrResult->ai_addrlen );
   assert( status == 0 && "error in bind()" );
   freeaddrinfo( addrResult );

   status = listen( sock, self.worldSize );
   assert( status == 0 && "error in listen()" );

   // accept, and spawn a new thread to handle the connection
   sockaddr clientAddr{};
   socklen_t socklen;
   while ( int fd = accept( sock, &clientAddr, &socklen ) )
      {
      pthread_t handlerThread;
      auto *args = new RemoteWorkerConnectionArgs{
         .self = static_cast<FilterWorker *>( inst ),
         .connFd = fd,
      };
      pthread_create( &handlerThread, nullptr, threadRemoteWorkerConnection,
                      args );
      pthread_detach( handlerThread );
      }
   assert( false && "accept() failed" );
   }

void *FilterWorker::threadRemoteWorkerConnection( void *p )
   {
   scb::UniquePointer args( static_cast<RemoteWorkerConnectionArgs *>( p ) );

   char buf[ MAX_URL_LEN + 1 ];
   int got = 0;
   while ( auto sz = recv( args->connFd, buf + got, sizeof( buf ) - got, 0 ) )
      {
      int start = 0;
      for ( int i = got;  i < got + sz;  i++ )
         if ( buf[ i ] == '\0' )
            {
            // found a null-terminated string at buf[start...i], where buf[i] is
            // NULL
            pthread_mutex_lock( &args->self->lockUnfilteredUrls );
            std::cout << "from remote: " << buf + start << std::endl;
            args->self->unfilteredUrls.pushBack( buf + start );
            pthread_mutex_unlock( &args->self->lockUnfilteredUrls );

            // notify the downstream
            sem_post( &args->self->semUnfilteredUrls );

            // resume searching from buf[i+1]
            start = i + 1;
            }

      // everything before buf[start] has been posted, that leaves us
      // (got + sz - start) bytes
      int bytesLeft = got + sz - start;
      // if we did consume some bytes
      if ( start != 0 )
         memmove( buf, buf + start, bytesLeft );
      got = bytesLeft;

      // if we got a lot of bytes without seeing a null-terminated string,
      // the other side must be misbehaving.
      if ( got == sizeof( buf ) )
         break;
      }

   close( args->connFd );
   return nullptr;
   }

[[noreturn]] void *FilterWorker::threadUnfilteredUrls( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   while ( true )
      {
      // wait until the queue has stuff in it
      sem_wait( &self.semUnfilteredUrls );

      pthread_mutex_lock( &self.lockUnfilteredUrls );
      ParsedUrl url( self.unfilteredUrls.front() );
      self.unfilteredUrls.popFront();
      pthread_mutex_unlock( &self.lockUnfilteredUrls );

      // filter by domain hash
      int rightfulHandler = url.Host.hash() % self.worldSize;
      if ( rightfulHandler != self.rank )
         {
         std::cout << "not our host: " << url.Host << std::endl;
         self.queueRemoteUrl( rightfulHandler, url.CompleteUrl );
         continue;
         }

      // filter by robots.txt
      if ( !self.allowedByRobotstxt( url ) )
         {
         std::cout << "not allowed by robots.txt: " << url.CompleteUrl
                   << std::endl;
         continue;
         }

      // filter by bloom filter
      if ( self.markIfNotSeen( url.CompleteUrl ) )
         {
         std::cout << "seen: " << url.CompleteUrl << std::endl;
         continue;
         }

      // all good! we can put that into filteredUrls
      pthread_mutex_lock( &self.lockFilteredUrls );
      self.filteredUrls.pushBack( std::move( url ) );
      pthread_mutex_unlock( &self.lockFilteredUrls );

      // notify the downstream
      sem_post( &self.semFilteredUrls );
      }
   }

void FilterWorker::queueRemoteUrl( int remote, const String &s )
   {
   // just extra sanity check
   if ( remote >= worldSize )
      return;

   pthread_mutex_lock( &lockRemoteWorkersSendBuffer );
   remoteWorkersSendBuffer[ remote ] += s;
   pthread_mutex_unlock( &lockRemoteWorkersSendBuffer );
   }

int FilterWorker::connectTo( const ParsedUrl &addr )
   {
   addrinfo *info = QueryHost( addr.Host.cstr(), addr.Port.cstr() );
   Defer freeAddrinfo(
      [ = ]()
      {
         freeaddrinfo( info );
      } );

   int sock = socket( info->ai_family, info->ai_socktype, info->ai_protocol );
   assert( sock != 0 && "error in socket()" );

   // connect socket
   int status = connect( sock, info->ai_addr, info->ai_addrlen );
   if ( status == -1 )
      throw std::runtime_error( strerror( errno ) );

   return sock;
   }

[[noreturn]] void *FilterWorker::threadRemoteUrls( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   // connect to remote workers
   assert( self.worldSize == self.remoteWorkerAddrs.size() );

   // initialize polling
   auto *events = new pollfd[ self.worldSize ];
   for ( int i = 0;  i < self.worldSize;  i++ )
      {
      events[ i ].fd = 0;
      while ( self.rank != i && events[ i ].fd == 0 )
         {
         try
            {
            events[ i ].fd = connectTo( self.remoteWorkerAddrs[ i ] );
            }
         catch ( std::exception &e )
            {
            std::cout << "cannot connect to peer " << i << " at "
                      << self.remoteWorkerAddrs[ i ].HostPort << ": "
                      << e.what() << std::endl;
            events[ i ].fd = 0;
            sleep( 1 );
            }
         }

      std::cout << "connected to peer " << i << " at "
                << self.remoteWorkerAddrs[ i ].HostPort << std::endl;
      }

   // set poll to listen for writable events
   for ( int i = 0;  i < self.worldSize;  i++ )
      events[ i ].events = POLLOUT;

   // poll loop
   while ( true )
      {
      int nEvents = poll( events, self.worldSize, -1 );
      assert( nEvents != -1 && "error in poll()" );
      for ( int i = 0;  nEvents != 0 && i < self.worldSize;  i++ )
         if ( events[ i ].revents )
            {
            nEvents--;
            if ( ( POLLHUP | POLLERR ) & events[ i ].revents )
               {
               // disconnected, try to reconnect
               // cleanup
               std::cerr << "connection broken with peer " << i << " at "
                         << self.remoteWorkerAddrs[ i ].HostPort << std::endl;
               close( events[ i ].fd );
               events[ i ].fd = 0;
               pthread_mutex_lock( &self.lockRemoteWorkersSendBuffer );
               self.remoteWorkersSendBuffer[ i ].clear();
               pthread_mutex_unlock( &self.lockRemoteWorkersSendBuffer );

               // reconnect
               while ( events[ i ].fd == 0 )
                  {
                  try
                     {
                     events[ i ].fd = connectTo( self.remoteWorkerAddrs[ i ] );
                     }
                  catch ( std::exception &e )
                     {
                     std::cout << "cannot connect to peer " << i << " at "
                               << self.remoteWorkerAddrs[ i ].HostPort << ": "
                               << e.what() << std::endl;
                     events[ i ].fd = 0;
                     sleep( 1 );
                     }
                  }

               std::cerr << "reconnected to peer " << i << " at "
                         << self.remoteWorkerAddrs[ i ].HostPort << std::endl;
               }
            else if ( POLLOUT & events[ i ].revents )
               {
               // if writable
               pthread_mutex_lock( &self.lockRemoteWorkersSendBuffer );
               auto &buf = self.remoteWorkersSendBuffer[ i ];
               // write as much from buf as possible
               int total = 0;
               while ( true )
                  {
                  int sent = send( events[ i ].fd, buf.data() + total,
                                   buf.size() - total, O_NONBLOCK );
                  // sent all
                  if ( sent == 0 )
                     break;
                  // even if it's not EAGAIN, we'll handle that on the next loop
                  if ( sent == -1 )
                     break;
                  total += sent;
                  }
               buf = buf.substr( total );
               pthread_mutex_unlock( &self.lockRemoteWorkersSendBuffer );
               }
            }
      }
   }

void *FilterWorker::threadOffloadUrls( void *p )
   {
   scb::UniquePointer urls( static_cast<Vector<ParsedUrl> *>( p ) );

   // get current time
   time_t timestamp = std::time( nullptr );
   tm *timeRepr = std::gmtime( &timestamp );
   char timeStr[ 1000 ];
   strftime( timeStr, sizeof( timeStr ), "%Y_%m_%d_%H_%M_%S_", timeRepr );
   scb::String filename =
      scb::String( timeStr ) + RandAlphaString( 6 ) + ".urls.txt";
   std::fstream fs( filename.cstr(), std::ios::out | std::ios::app );

   std::cout << "offloading " << urls->size() << " URLs to " << filename
             << std::endl;
   for ( ParsedUrl &url : *urls )
      fs << url.CompleteUrl << '\n';

   return nullptr;
   }

void *FilterWorker::threadFilteredUrls( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );
   while ( true )
      {
      // number of URLs issued for this host in this tick
      pthread_mutex_lock( &self.lockFilteredUrls );

      // offload some urls to disk if we have too many in memory
      if ( self.filteredUrls.size() >
           PENDING_LINKS_OFFLOAD_FACTOR * MAX_PENDING_LINKS )
         {
         // make a vector of URLs to offload
         auto *offload = new Vector<ParsedUrl>;
         while ( self.filteredUrls.size() > MAX_PENDING_LINKS )
            {
            offload->pushBack( std::move( self.filteredUrls.front() ) );
            self.filteredUrls.popFront();
            }
         pthread_t th;
         if ( pthread_create( &th, nullptr, threadOffloadUrls, offload ) != 0 )
            {
            // if new thread failed, just do the offloading here
            threadOffloadUrls( offload );
            delete offload;
            }
         else
            pthread_detach( th );
         }

      // count how many times have we hit this host in this tick
      Counter hostCounter;

      for ( int i = 0, loop = self.filteredUrls.size();  i < loop;  i++ )
         {
         ParsedUrl url = std::move( self.filteredUrls.front() );
         self.filteredUrls.popFront();

         // this should not happen, but check just in case
         if ( url.CompleteUrl.size() > MAX_URL_LEN )
            {
            std::cout << "URL too long: " << url.CompleteUrl << std::endl;
            continue;
            }

         // if rate limit for this host reached, requeue
         if ( hostCounter.Get( url.Host ) >= self.getHostRateLimit( url.Host ) )
            {
            self.filteredUrls.pushBack( url );
            std::cout << "rate limit hit for: " << url.CompleteUrl << std::endl;
            continue;
            }

         // if mqueue blocked, wait until the next tick
         if ( mq_send( self.qFrontier, url.CompleteUrl.data(),
                       url.CompleteUrl.size(), 0 ) == -1 )
            {
            if ( errno == EAGAIN )
               {
               self.filteredUrls.pushBack( url ); // requeue
               std::cout
                  << "frontier mqueue blocked: crawling isn't fast enough"
                  << std::endl;
               break; // wait until the next tick
               }
            else
               assert( false && "mq_send() failed" );
            }

         // update rate count
         hostCounter.Increment( url.Host );

         std::cout << "posted to frontier: " << url.CompleteUrl << std::endl;
         }

      pthread_mutex_unlock( &self.lockFilteredUrls );

      usleep( 200 * 1000 ); // wait until next tick
      }
   }

bool FilterWorker::markIfNotSeen( const String &url )
   {
   return bloomfilter.Insert( url );
   }

int FilterWorker::getHostRateLimit( const String &host )
   {
   return 1; // TODO: read alexa
   }

Robotstxt *const FilterWorker::ROBOTS_TXT_PENDING =
   reinterpret_cast<Robotstxt *const>( 1 );
Robotstxt *const FilterWorker::DummyRobotstxt =
   new Robotstxt( "User-agent: *\n" );

bool FilterWorker::allowedByRobotstxt( const ParsedUrl &url )
   {
   pthread_mutex_lock( &lockRobotstxtMap );

   // if no robots.txt yet, fetch and set
   if ( !robotstxtMap.count( url.Host ) )
      {
      std::cout << "no robots.txt for " << url.Host << std::endl;
      robotstxtMap[ url.Host ] = ROBOTS_TXT_PENDING;
      pthread_mutex_unlock(
         &lockRobotstxtMap ); // unlock to allow other threads to proceed

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
      }

   // now that we can be sure that we have a robots.txt
   auto *robotstxt = robotstxtMap[ url.Host ];

   // while pending, sleep until it is ready
   while ( robotstxt == ROBOTS_TXT_PENDING )
      {
      // unlock to allow other threads to proceed while we wait
      pthread_mutex_unlock( &lockRobotstxtMap );
      std::cout << "waiting for robots.txt of " << url.Host << std::endl;
      usleep( 500 * 1000 );
      pthread_mutex_lock( &lockRobotstxtMap );
      robotstxt = robotstxtMap[ url.Host ];
      }

   bool allowed = robotstxt->Allow( url.CompleteUrl );
   pthread_mutex_unlock( &lockRobotstxtMap );
   return allowed;
   }

FilterWorker::FilterWorker( const char *remoteConfigFile, int rk,
                            const char *bloomFile )
    : bloomfilter( bloomFile )
   {
   // according to SSL document, we should ignore this
   signal( SIGPIPE, SIG_IGN );

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
   assert( sem_init( &semFilteredUrls, 0, 0 ) != -1 && "sem_init() failed" );
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

int main( int argc, char **argv )
   {

   if ( argc != 4 )
      {
      fprintf( stderr, "Usage: %s [remote-config] [bloom-file] [rank]\n",
               argv[ 0 ] );
      return 1;
      }

   FilterWorker worker( argv[ 1 ], strtol( argv[ 3 ], nullptr, 10 ),
                        argv[ 2 ] );
   worker.run();
   }