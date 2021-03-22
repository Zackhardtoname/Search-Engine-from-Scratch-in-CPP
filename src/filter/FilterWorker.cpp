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
#include <cassert>
#include <iostream>
#include <mqueue.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <slapi-plugin.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

using scb::String;
using scb::Vector;

void FilterWorker::run()
   {
   // open the queues
   mqd_t frontier = mq_open( FRONTIER_MQ, O_WRONLY | O_NONBLOCK );
   assert( frontier != -1 && "mq_open() failed" );

   mqd_t newLinks = mq_open( NEW_LINKS_QUEUE, O_RDONLY | O_NONBLOCK );
   assert( newLinks != -1 && "mq_open() failed" );

   pthread_create( &pthNewLinksMqueue, nullptr, threadNewLinksMqueue, this );
   pthread_create( &pthRemoteWorkers, nullptr, threadRemoteWorkers, this );

   for ( pthread_t &pth : pthUnfilteredUrls )
      pthread_create( &pth, nullptr, threadUnfilteredUrls, this );
   }

void *FilterWorker::threadNewLinksMqueue( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   char buf[ MAX_URL_LEN + 1 ];
   while ( true )
      {
      ssize_t sz = mq_receive( self.qNewLinks, buf, MAX_URL_LEN, nullptr );
      assert( sz != -1 && "mq_receive() failed" );

      buf[ sz ] = '\0';

      pthread_mutex_lock( &self.lockUnfilteredUrls );
      self.unfilteredUrls.pushBack( buf );
      pthread_mutex_unlock( &self.lockUnfilteredUrls );

      // notify the downstream
      sem_post( &self.semUnfilteredUrls );
      }
   }

void *FilterWorker::threadRemoteWorkers( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   // create, bind, and listen socket
   char portStr[ 20 ];
   sprintf( portStr, "%d", self.port );
   addrinfo *addrResult;
   try
      {
      addrResult = QueryHost( self.hostname, portStr );
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
            args->self->unfilteredUrls.pushBack( buf + start );
            pthread_mutex_unlock( &args->self->lockUnfilteredUrls );

            // notify the downstream
            sem_post( &args->self->semUnfilteredUrls );

            // resume searching from buf[i+1]
            start = i + 1;
            }

      // everything before buf[start] has been posted, that leaves us (got + sz
      // - start) bytes
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
         self.queueRemoteUrl( rightfulHandler, url.CompleteUrl );
         continue;
         }

      // filter by robots.txt
      if ( !self.allowedByRobotstxt( url ) )
         continue;

      // filter by bloom filter
      if ( self.markIfNotSeen( url.CompleteUrl ) )
         continue;

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
   if ( status == 0 )
      throw std::runtime_error( "cannot connect()" );
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
            std::cerr << "cannot connect to peer " << i << " at "
                      << self.remoteWorkerAddrs[ i ].HostPort << ": "
                      << e.what() << std::endl;
            events[ i ].fd = 0;
            sleep( 1 );
            }
         }
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
                     std::cerr << "cannot connect to peer " << i << " at "
                               << self.remoteWorkerAddrs[ i ].HostPort << ": "
                               << e.what() << std::endl;
                     events[ i ].fd = 0;
                     sleep( 1 );
                     }
                  }
               }
            else if ( POLLOUT & events[ i ].revents )
               {
               // if writable
               pthread_mutex_lock( &self.lockRemoteWorkersSendBuffer );
               auto &buf = self.remoteWorkersSendBuffer[ i ];
               // write as much from buf as possible
               int total = 0;
               while ( int sent = send( events[ i ].fd, buf.data() + total,
                                        buf.size() - total, O_NONBLOCK ) )
                  total += sent;
               buf = buf.substr( total );
               pthread_mutex_unlock( &self.lockRemoteWorkersSendBuffer );
               }
            }
      }
   }

void *FilterWorker::threadFilteredUrls( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );
   while ( true )
      {
      // number of URLs issued for this host in this tick
      pthread_mutex_lock( &self.lockFilteredUrls );

      // count how many times have we hit this host in this tick
      Counter<String> hostCounter;

      for ( int i = 0, loop = self.filteredUrls.size();  i < loop;  i++ )
         {
         ParsedUrl url = std::move( self.filteredUrls.front() );
         self.filteredUrls.popFront();

         // this should not happen, but check just in case
         if ( url.CompleteUrl.size() > MAX_URL_LEN )
            continue;

         // if rate limit for this host reached, requeue
         if ( hostCounter.Get( url.Host ) >= self.getHostRateLimit( url.Host ) )
            {
            self.filteredUrls.pushBack( std::move( url ) );
            continue;
            }

         // if mqueue blocked, wait until the next tick
         if ( mq_send( self.qFrontier, url.CompleteUrl.data(),
                       url.CompleteUrl.size(), 0 ) == -1 )
            {
            if ( errno == EAGAIN )
               {
               self.filteredUrls.pushBack( std::move( url ) ); // requeue
               break; // wait until the next tick
               }
            else
               assert( false && "mq_send() failed" );
            }

         // update rate count
         hostCounter.Increment( url.Host );

         std::cout << "posted: " << url.CompleteUrl << std::endl;
         }

      pthread_mutex_unlock( &self.lockFilteredUrls );

      usleep( 200 * 1000 ); // wait until next tick
      }
   }

bool FilterWorker::markIfNotSeen( const String &url )
   {
   }
