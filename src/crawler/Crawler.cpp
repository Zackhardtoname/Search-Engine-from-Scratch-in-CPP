#include "../stl/RAII.h"
#include "../stl/utils.h"
#include <cassert>
#include <iostream>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "../stl/UniquePointer.h"
#include "Crawler.h"
#include "ParsedHttpResponse.h"
#include "ParsedUrl.h"

// TODO: make it so that we abort on lengthy response, and/or timeout
// TODO: setup build file for test
// TODO: make it so that we abort on binary (i.e. non html or text) response

std::string getOpenSSLError()
   {
   BIO *bio = BIO_new( BIO_s_mem() );
   ERR_print_errors( bio );
   char *buf;
   size_t len = BIO_get_mem_data( bio, &buf );
   std::string ret( buf, len );
   BIO_free( bio );
   return ret;
   }

String HttpGet( const ParsedUrl &url )
   {
   // Create and connect a socket.
   addrinfo *addr =
      QueryHost( url.Host.cstr(), url.Port.length() == 0 ? url.Service.cstr()
                                                         : url.Port.cstr() );
   Defer freeAddrinfo(
      [ & ]()
      {
         freeaddrinfo( addr );
      } );

   int sockFd = socket( addr->ai_family, addr->ai_socktype, addr->ai_protocol );
   assert( sockFd != -1 );
   Defer closeSocket(
      [ & ]()
      {
         close( sockFd );
      } );

   int status = connect( sockFd, addr->ai_addr, addr->ai_addrlen );
   if ( status != 0 )
      throw NetworkException( "cannot connect()" );

   // Prepare a GET message
   String getRequest =
      String( "GET " ) + url.Path +
      " HTTP/1.1\r\n"
      "Host: " +
      url.Host + "\r\n" +
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
      "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.190 "
      "Safari/537.36 zackLt@umich.edu\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept: text/html\r\n"
      "Accept-Encoding: identity\r\n"
      "Connection: close\r\n\r\n";

   String response;
   char buffer[ 10240 ];
   if ( url.Service == "http" )
      {
      for ( int sent = 0;  sent < getRequest.size();  )
         {
         int n = send( sockFd, getRequest.data() + sent,
                       getRequest.size() - sent, 0 );
         if ( n <= 0 )
            throw NetworkException();
         sent += n;
         }
      shutdown( sockFd, SHUT_WR );

      int got;
      while ( ( got = recv( sockFd, buffer, sizeof( buffer ), 0 ) ) > 0 )
         response += String( buffer, got );
      }
   else if ( url.Service == "https" )
      {
      SSL_library_init();
      SSL_CTX *ctx = SSL_CTX_new( TLS_client_method() );
      SSL *ssl = SSL_new( ctx );

      if ( SSL_set_fd( ssl, sockFd ) == 0 )
         throw NetworkException();
      if ( SSL_connect( ssl ) != 1 )
         throw NetworkException();

      Defer deferDestructSSL(
         [ & ]()
         {
            SSL_shutdown( ssl );
            SSL_free( ssl );
            SSL_CTX_free( ctx );
         } );

      for ( int sent = 0;  sent < getRequest.size();  )
         {
         int n = SSL_write( ssl, getRequest.data() + sent,
                            getRequest.size() - sent );
         if ( n <= 0 )
            {
            std::cout << getOpenSSLError() << std::endl;
            throw NetworkException();
            }
         sent += n;
         }

      int got;
      while ( ( got = SSL_read( ssl, buffer, sizeof( buffer ) ) ) > 0 )
         response += String( buffer, got );
      }
   else
      {
      throw std::runtime_error( "protocol not supported" );
      }

   return response;
   }

ParsedHttpResponse FetchUrl( String url )
   {
   // Webpage that redirects too many times are either ads-clickfarms, or
   // malfunctioning. We only redirect for a generous MAX_REDIRECTION number
   // of times and stop following after that.
   for ( int nRedirect = 0;  nRedirect < MAX_REDIRECTION;  nRedirect++ )
      {
      ParsedUrl parsedUrl( url );
      String resp = HttpGet( parsedUrl );
      ParsedHttpResponse parsedResp( resp );
      if ( parsedResp.StatusCode == "200" )
         return parsedResp;
      if ( parsedResp.StatusCode == "301" || parsedResp.StatusCode == "302" ||
           parsedResp.StatusCode == "307" || parsedResp.StatusCode == "308" )
         url = parsedUrl.Normalize( parsedResp.Location );
      else
         {
         printf( "status: %s\n", parsedResp.StatusCode.cstr() );
         throw HttpRequestError( "status code not handled" );
         }
      // TODO: should we, and how do we, handle HTTP 50x?
      }

   throw TooManyRedirectionError();
   }
