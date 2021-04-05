//
// Created by youh on 4/3/21.
//

#include <csignal>
#include <netdb.h>
#include <unistd.h>

#include "../stl/utils.h"
#include "FilterWorker.h"

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
         .ConnFd = fd,
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
   while ( auto sz = recv( args->ConnFd, buf + got, sizeof( buf ) - got, 0 ) )
      {
      int start = 0;
      for ( int i = got;  i < got + sz;  i++ )
         if ( buf[ i ] == '\0' )
            {
            // found a null-terminated string at buf[start...i], where buf[i] is
            // NULL
#ifndef NDEBUG
            std::cout << "from remote: " << buf + start << std::endl;
#endif
            args->self->handleUnfilteredLink( String( buf + start, buf + i ) );

            // resume searching from buf[i+1]
            start = i + 1;
            }

      // everything before buf[start] has been posted, that leaves us
      // (got + sz - start) bytes
      int bytesLeft = got + sz - start;
      // if we did consume some bytes, left shift the remaining bytes
      if ( start != 0 )
         memmove( buf, buf + start, bytesLeft );
      got = bytesLeft;

      // if we got a lot of bytes without seeing a null-terminated string,
      // the other side must be misbehaving. Close connection.
      if ( got == sizeof( buf ) )
         break;
      }

   close( args->ConnFd );
   return nullptr;
   }
