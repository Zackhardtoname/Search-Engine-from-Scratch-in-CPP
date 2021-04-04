//
// Created by youh on 4/3/21.
//

#include <cstdlib>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "../stl/DevOps.h"
#include "../stl/RAII.h"
#include "../stl/utils.h"
#include "FilterWorker.h"

void FilterWorker::queueRemoteUrl( int remote, const String &url )
   {
   // just extra sanity check
   if ( remote >= worldSize )
      return;

   pthread_mutex_lock( &lockRemoteWorkersSendBuffer );
   remoteWorkersSendBuffer[ remote ] += url;
   remoteWorkersSendBuffer[ remote ].pushBack( 0 ); // null-terminate
   pthread_mutex_unlock( &lockRemoteWorkersSendBuffer );

   // notify that new content is available in sendBuffer
   uint64_t avail = 1;
   write( sendBufferAvailEventFd, &avail, sizeof( avail ) );
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

   assert( self.worldSize == self.remoteWorkerAddrs.size() );

   // connect to remote workers
   // initialize the connections for polling
   Vector<pollfd> events( self.worldSize );
   for ( int i = 0;  i < self.worldSize;  i++ )
      // for our slot, put in an eventfd to get notification from other
      // threads about change to remoteWorkersSendBuffer
      if ( i == self.rank )
         {
         // don't use EFD_SEMAPHORE here, because multiple write events
         // is handled in a single iteration.
         self.sendBufferAvailEventFd = events[ i ].fd = eventfd( 0, 0 );
         assert( events[ i ].fd != -1 && "eventfd() failed" );
         }
      else
         {
         events[ i ].fd = 0;
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

         std::cout << "connected to peer " << i << " at "
                   << self.remoteWorkerAddrs[ i ].HostPort << std::endl;
         }

   // poll loop
   while ( true )
      {
      // set poll to listen for writable events only on those workers
      // whose buffers are not empty
      pthread_mutex_lock( &self.lockRemoteWorkersSendBuffer );
      for ( int i = 0;  i < self.worldSize;  i++ )
         if ( i != self.rank && !self.remoteWorkersSendBuffer[ i ].empty() )
            events[ i ].events = POLLOUT;
         else
            events[ i ].events = 0;

      // don't forget to listen for the eventfd
      events[ self.rank ].events = POLLIN;
      pthread_mutex_unlock( &self.lockRemoteWorkersSendBuffer );

      int nEvents = poll( events.data(), self.worldSize, -1 );
      assert( nEvents != -1 && "error in poll()" );
      for ( int i = 0;  nEvents != 0 && i < self.worldSize;  i++ )
         if ( events[ i ].revents )
            {
            nEvents--;
            if ( ( POLLHUP | POLLERR ) & events[ i ].revents )
               {
               // remote worker disconnected, try to reconnect
               std::cerr << "connection broken with peer " << i << " at "
                         << self.remoteWorkerAddrs[ i ].HostPort << std::endl;

               events[ i ].fd = 0;
               // reconnect
               // if we blocked every other threads
               bool blocked = false;
               for ( int nRetries = 0;  events[ i ].fd == 0;  nRetries++ )
                  {
                  // if we have retried too many times, block and warn devops
                  if ( nRetries > REMOTE_WORKER_DOWN_RETRY_LIMIT && !blocked )
                     {
                     self.block();
                     blocked = true;
                     // issue a warning
                     String errMsg = self.remoteWorkerAddrs[ i ].CompleteUrl +
                                     " lost connection to " +
                                     self.remoteWorkerAddrs[ i ].CompleteUrl;
                     DiscordAlert( errMsg );
                     }
                  try
                     {
                     events[ i ].fd = connectTo( self.remoteWorkerAddrs[ i ] );
                     }
                  catch ( std::exception &e )
                     {
                     std::cout << "cannot reconnect to peer " << i << " at "
                               << self.remoteWorkerAddrs[ i ].HostPort << ": "
                               << e.what() << std::endl;
                     events[ i ].fd = 0;
                     usleep( 500 * 1000 );
                     }
                  }

               std::cerr << "reconnected to peer " << i << " at "
                         << self.remoteWorkerAddrs[ i ].HostPort << std::endl;
               if ( blocked )
                  self.unblock();
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
            else if ( POLLIN & events[ i ].revents )
               {
               assert( i == self.rank && "unexpected POLLIN event" );
               uint64_t buf;
               read( events[ self.rank ].fd, &buf, sizeof( buf ) );
               // nothing else to do, since this is just to trigger the loop
               }
            }
      }
   }
