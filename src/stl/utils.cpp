//
// Created by youh on 2/20/21.
//

#include <cassert>
#include <string>
#include <sys/stat.h>

#include "utils.h"

using scb::String;

String AddrToString( const sockaddr_storage &addr, socklen_t addrLen )
   {
   std::string host( MAX_HOSTNAME_LEN, '\0' );
   std::string service( MAX_HOSTNAME_LEN, '\0' );
   int status = getnameinfo( reinterpret_cast<const sockaddr *>( &addr ),
                             addrLen, (char *)( host.data() ), MAX_HOSTNAME_LEN,
                             (char *)service.data(), MAX_HOSTNAME_LEN,
                             NI_NAMEREQD | NI_NUMERICSERV );
   if ( status == 0 )
      return String( host.c_str() ) + ":" + String( service.c_str() );
   return gai_strerror( status );
   }

void SetTCPNoDelay( int fd )
   {
   int yes = 1;
   int status = setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof( int ) );
   assert( status == 0 && "error in setsockopt()" );
   }

addrinfo *QueryHost( const char *host, const char *service )
   {
   addrinfo hint = { /*ai_flag*/ 0,
                     /*ai_family*/ AF_INET,
                     /*ai_socktype*/ SOCK_STREAM,
                     /*ai_protocol*/ IPPROTO_TCP,
                     /*ai_addrlen*/ sizeof( addrinfo ),
                     /*ai_addr*/ nullptr,
                     /*ai_canonname*/ nullptr,
                     /*ai_next*/ nullptr };
   addrinfo *addrResult = nullptr;
   int status = getaddrinfo( host, service, &hint, &addrResult );
   if ( status != 0 )
      {
      printf( "getaddrinfo(%s:%s): %s\n", host, service, strerror( status ) );
      throw std::runtime_error( "getaddrinfo() error" );
      }
   return addrResult;
   }

void SetNonBlocking( int fd )
   {
   int flags = fcntl( fd, F_GETFL, 0 );
   assert( flags != -1 && "error in fcntl()" );
   assert( fcntl( fd, F_SETFL, flags | O_NONBLOCK ) == 0 &&
           "error in fcntl()" );
   }

scb::String RandAlphaString( int len )
   {
   scb::String str;
   for ( int i = 0;  i < len;  i++ )
      str.pushBack( 'a' + rand() % 26 );
   return str;
   }

size_t FileSize( int fd )
   {
   struct stat fileInfo
      {
      };
   fstat( fd, &fileInfo );
   return fileInfo.st_size;
   }
