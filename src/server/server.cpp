// Usage:  server port rootdirectory

// Compile with g++ -pthread server.cpp -o server
// To run under WSL (Windows Subsystem for Linux), may have to elevate
// with sudo if the bind fails.

// It does not look for default index.htm or similar
// files.  If it receives a GET request on a directory, it will refuse
// it, returning an HTTP 403 error, access denied.

// It also does not support HTTP Connection: keep-alive requests and
// will close the socket at the end of each response.  This is a
// perf issue, forcing the client browser to reconnect for each
// request and a candidate for improvement.

#include "../stl/Filesystem.h"
#include "../stl/String.h"
#include "../stl/Vector.h"
#include "../stl/utils.h"
#include "HTTPEscapeCodes.h"
#include "Mime.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define backlog 1000

using std::cerr;
using std::cout;
using std::endl;

// Root directory for the website, taken from argv[ 2 ].
// (Yes, a global variable since it never changes.)
char *RootDirectory;

int HexLiteralCharacter( char c )
   {
   // If c contains the Ascii code for a hex character, return the
   // binary value; otherwise, -1.

   int i;

   if ( '0' <= c && c <= '9' )
      i = c - '0';
   else if ( 'a' <= c && c <= 'f' )
      i = c - 'a' + 10;
   else if ( 'A' <= c && c <= 'F' )
      i = c - 'A' + 10;
   else
      i = -1;

   return i;
   }

scb::String UnencodeUrlEncoding( scb::String &path )
   {
   // Unencode any %xx encodings of characters that can't be
   // passed in a URL.

   // (Unencoding can only shorten a string or leave it unchanged.
   // It never gets longer.)

   const char *start = path.cstr(), *from = start;
   scb::String result;
   char c, d;

   while ( ( c = *from++ ) != 0 )
      if ( c == '%' )
         {
         c = *from;
         if ( c )
            {
            d = *++from;
            if ( d )
               {
               int i, j;
               i = HexLiteralCharacter( c );
               j = HexLiteralCharacter( d );
               if ( i >= 0 && j >= 0 )
                  {
                  from++;
                  result += (char)( i << 4 | j );
                  }
               else
                  {
                  // If the two characters following the %
                  // aren't both hex digits, treat as
                  // literal text.
                  result += '%';
                  from--;
                  }
               }
            }
         }
      else
         result += c;

   return result;
   }

bool SafePath( const char *path )
   {
   // The path must start with a /.
   if ( *path != '/' )
      return false;

   // TODO:  Return false for any path containing ..
   // segments that attempt to go higher than the root
   // directory for the website.
   return true;
   }

void AccessDenied( int talkSocket )
   {
   const char accessDenied[] = "HTTP/1.1 403 Access Denied\r\n"
                               "Content-Length: 0\r\n"
                               "Connection: close\r\n\r\n";

   cout << accessDenied << endl;
   send( talkSocket, accessDenied, sizeof( accessDenied ) - 1, 0 );
   }

void FileNotFound( int talkSocket )
   {
   const char fileNotFound[] = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Length: 0\r\n"
                               "Connection: close\r\n\r\n";

   cout << fileNotFound << endl;
   send( talkSocket, fileNotFound, sizeof( fileNotFound ) - 1, 0 );
   }

void *Talk( void *talkSocket )
   {
   // Look for a GET message, then reply with the
   // requested file.

   // Cast from void * to int * to recover the talk socket id
   // then free the heap usage as soon as possible in case exception happens
   // later
   cout << "received connection at socket " << talkSocket << ":" << endl;
   int talkSocketFD = *(int *)talkSocket;
   free( talkSocket );

   // Read the request from the socket and parse it to extract
   // the action and the path, unencoding any %xx encodings.
   char buffer[ 1024 ];
   int bytes;

   // Assuming a single client won't send a ton of traffic
   if ( ( bytes = recv( talkSocketFD, buffer, sizeof( buffer ), 0 ) ) > 0 )
      cout << buffer << endl;

   scb::String buf( buffer );
   // action must be "GET" and
   // the path must be safe.
   int requestStartLoc = 4;
   int protocalLoc = buf.find( "HTTP" );
   scb::String queryHint = "/search";
   scb::String end_char = "\r\n";

   if ( buf.substr( 0, 3 ) == "GET" && protocalLoc != scb::String::npos )
      {
      scb::String requested =
         buf.substr( requestStartLoc,
                     protocalLoc - requestStartLoc - 1 ); // 1 for whitespace
      scb::String processedPath = ( UnencodeUrlEncoding( requested ) );

      if ( processedPath == "/" )
         {
         processedPath = "/index.html";
         }

      if ( !SafePath( processedPath.cstr() ) )
         // If the path refers to a directory
         AccessDenied( talkSocketFD );
      else if ( !strncmp( requested.cstr(), queryHint.cstr(),
                          queryHint.length() ) )
         // Get search results and return them
         {

         // Begin by extracting the query from the HTTP request header
         scb::String endOfQuery = " HTTP";
         // 18 refers to the length of the string "GET /search?query=" which will
         // always be the same length
         int queryLength = buf.find( endOfQuery, 0 ) - 18;
         scb::String query =
            buf.substr( 18, queryLength );

         // Convert all of the HTTP symbols to the correct symbols in the query
         URLDecoder decode;
         query = decode.escapeCodesToString(query);


         // Call the query parser and ranker to recieve the results
         scb::Vector<scb::String> allMatches(20);

         // Create a json string that is formatted appropriately to send to the
         // client. Currently, we send the top 20 results (hence the 20
         // iterations in the for loop) but this is subject to change depending
         // on what we want to do
         scb::String json = "{\n\t\"pages\":\t[\n";
         for ( int i = 0;  i < 3;  ++i )
            {
            json += "\t\t{\n\t\t\t\"title\": \"";
            json += "Title\",\n";
            json += "\t\t\t\"url\": \"";
            json += "URL\",\n";
            json += "\t\t\t\"snippet\": \"";
            json += "Snippet\",\n";
            json += "\t\t\t\"rank\": \"";

            // Round the rank up or down to the nearest whole number(integer)
            // value
            json += std::to_string( 5 ).c_str();
            json += "\"\n";
            if ( i == 2 )
               json += "\t\t}\n";
            else
               json += "\t\t},\n";
            }
         json += "\t]\n}\n";

         // header
         scb::String msg( "HTTP/1.1 200 OK" );
         msg += end_char;
         msg += "Content-Length: ";
         msg += json.length();
         msg += end_char;
         msg += "Content-Type: ";
         msg += "application/json";
         msg += end_char;
         msg += end_char;

         // body
         msg += json;
         std::cout << "query: " << query.cstr() << endl;
         std::cout << json.cstr() << endl;

         // Send the message with the top n results to the client
         send( talkSocketFD, msg.cstr(), msg.size(), 0 );
         }
      else
         {
         // If the path refers to a file, write it to the socket.
         char *fileStart;
         char *fileEnd;
         processedPath =
            scb::String( RootDirectory ) + "/" + processedPath.substr( 1 );
         const char *processedPathCStr = processedPath.cstr();
         int mapRes = mapfile( &processedPathCStr, &fileStart, &fileEnd );
         if ( mapRes < 1 )
            {
            FileNotFound( talkSocketFD );
            }
         else
            {
            // header
            scb::String msg( "HTTP/1.1 200 OK" );
            msg += end_char;
            msg += "Content-Length: ";
            msg += fileEnd - fileStart;
            msg += end_char;
            msg += "Content-Type: ";
            msg += getMimetype( processedPathCStr );
            msg += end_char;
            msg += end_char;

            // body
            msg += fileStart;

            send( talkSocketFD, msg.cstr(), msg.size(), 0 );
            }
         }
      }
   else
      {
      AccessDenied( talkSocketFD );
      }

   // Close the socket and return nullptr.
   close( talkSocketFD );
   return nullptr;
   }

int main( int argc, char **argv )
   {
   if ( argc != 3 )
      {
      cerr << "Usage:  " << argv[ 0 ] << " port rootdirectory" << endl;
      return 1;
      }

   int port = atoi( argv[ 1 ] );
   RootDirectory = argv[ 2 ];

   // Discard any trailing slash.  (Any path specified in
   // an HTTP header will have to start with /.)
   char *r = RootDirectory;
   if ( *r )
      {
      do
         r++;
      while ( *r );
      r--;
      if ( *r == '/' )
         *r = 0;
      }

   // We'll use two sockets, one for listening for new
   // connection requests, the other for talking to each
   // new client.
   int listenSocket, talkSocket;

   // Create socket address structures to go with each
   // socket.
   struct sockaddr_in listenAddress, talkAddress;
   socklen_t listenAddressLength = sizeof( listenAddress );
   memset( &listenAddress, 0, sizeof( listenAddress ) );
   memset( &talkAddress, 0, sizeof( talkAddress ) );

   // Fill in details of where we'll listen.
   // We'll use the standard internet family of protocols.
   listenAddress.sin_family = AF_INET;

   // htons( ) transforms the port number from host (our)
   // byte-ordering into network byte-ordering (which could
   // be different).
   listenAddress.sin_port = htons( port );

   // INADDR_ANY means we'll accept connections to any IP
   // assigned to this machine.
   listenAddress.sin_addr.s_addr = htonl( INADDR_ANY );

   // Create the listenSocket, specifying that we'll r/w
   // it as a stream of bytes using TCP/IP.
   listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

   int enable = 1;
   if ( setsockopt( listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable,
                    sizeof( int ) ) < 0 )
      std::cout << "setsockopt(SO_REUSEADDR) failed";

   // Bind the listen socket to the IP address and protocol
   // where we'd like to listen for connections.
   int bindRes = bind( listenSocket, (struct sockaddr *)&listenAddress,
                       sizeof( listenAddress ) );
   if ( bindRes < 0 )
      cout << "bind failed " << bindRes << endl;

   // Begin listening for clients to connect to us.
   // The second argument to listen( ) specifies the maximum
   // number of connection requests that can be allowed to
   // stack up waiting for us to accept them before Linux
   // starts refusing or ignoring new ones.
   //
   // SOMAXCONN is a system-configured default maximum socket
   // queue length.  (Under WSL Ubuntu, it's defined as 128
   // in /usr/include/x86_64-linux-gnu/bits/socket.h.)
   if ( listen( listenSocket, backlog ) < 0 )
      cout << "listen failed" << endl;

   // Accept each new connection and create a thread to talk with
   // the client over the new talk socket that's created by Linux
   // when we accept the connection.

   // Create and detach a child thread to talk to the
   // client using pthread_create and pthread_detach.

   // When creating a child thread, you get to pass a void *,
   // usually used as a pointer to an object with whatever
   // information the child needs.

   // The talk socket is passed on the heap rather than with a
   // pointer to the local variable because we're going to quickly
   // overwrite that local variable with the next accept( ).  Since
   // this is multithreaded, we can't predict whether the child will
   // run before we do that.  The child will be responsible for
   // freeing the resource.  We do not wait for the child thread
   // to complete.
   //
   // (A simpler alternative in this particular case would be to
   // caste the int talksocket to a void *, knowing that a void *
   // must be at least as large as the int.  But that would not
   // demonstrate what to do in the general case.)
   while ( ( talkSocket =
                accept( listenSocket, (struct sockaddr *)( &listenAddress ),
                        (socklen_t *)&listenAddressLength ) ) &&
           talkSocket != -1 )
      {
      pthread_t child;
      pthread_create( &child, nullptr, Talk, new int( talkSocket ) );
      pthread_detach( child );
      }

   close( listenSocket );
   }
