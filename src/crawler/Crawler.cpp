#include "../stl/RAII.h"
#include "../stl/utils.h"
#include <cassert>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../stl/UniquePointer.h"
#include "Crawler.h"
#include "ParsedHttpResponse.h"
#include "ParsedUrl.h"

String FetchUrl( const String &url )
   {
   int contentFd[ 2 ];
   assert( pipe( contentFd ) == 0 && "pipe() failed" );
   int infoFd[ 2 ];
   assert( pipe( infoFd ) == 0 && "pipe() failed" );

   Defer closePipe(
      [ & ]()
      {
         close( contentFd[ 0 ] );
         close( infoFd[ 0 ] );
      } );

   ForkedChild curlProc;
   if ( curlProc.IsChild() )
      {
      // child
      dup2( contentFd[ 1 ], STDOUT_FILENO ); // redirect stdout to content
      close( contentFd[ 0 ] ); // now that STDOUT_FILENO points to the pipe,
      close(
         contentFd[ 1 ] ); // we can close the two original file descriptors.
      dup2( infoFd[ 1 ], STDERR_FILENO ); // redirect stderr to info
      close( infoFd[ 0 ] );
      close( infoFd[ 1 ] );

      execl(
         "/usr/bin/curl", "curl",
         "--disable",               // don't read .curlrc
         "--fail",                  // error whenever not HTTP 200 OK
         "--silent",                // no progress bar to mess up stderr
         "--max-filesize", "5m",    // abort on files larger than this
         "--location",              // follow redirections
         "--max-redirs", "10",      // at most 10 times
         "--connect-timeout", "10", // 10 sec connect timeout
         "--max-time", "30",        // 30 sec total timeout
         "--insecure",              // allow downgrade
         "--user-agent",
         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
         "like Gecko) Chrome/88.0.4324.190 Safari/537.36 zacklt@umich.edu",
         "--retry", "1", // retry once if transient failure
         "--write-out",
         "%{stderr}%{content_type} code: %{http_code}", // put these to stderr
         url.cstr(), nullptr );
      assert( !"execl() failed" );
      }

   // parent
   // close the write sides of the pipe, so that we are not blocked
   // by living pipe when the child exits
   close( contentFd[ 1 ] );
   close( infoFd[ 1 ] );

   // read stdout output
   scb::String content;
   char buf[ 2000 ];
   while ( true )
      {
      int got = read( contentFd[ 0 ], buf, sizeof( buf ) );
      if ( got == 0 )
         break;
      if ( got == -1 )
         throw std::runtime_error( strerror( errno ) );
      content += String( buf, got );
      }

   // read stderr output
   scb::String meta;
   while ( true )
      {
      int got = read( infoFd[ 0 ], buf, sizeof( buf ) );
      if ( got == 0 )
         break;
      if ( got == -1 )
         throw std::runtime_error( strerror( errno ) );
      meta += String( buf, got );
      }

   if ( !meta.empty() && !meta.hasPrefixCaseInsensitive( "text" ) )
      {
      scb::String errMsg = "not a text/* file, actually ";
      errMsg += meta;
      throw std::runtime_error( errMsg.cstr() );
      }

   int retCode = curlProc.Wait();
   if ( retCode != 0 )
      {
      scb::String errMsg = "curl error: ";
      errMsg += std::to_string( retCode ).c_str();
      errMsg += " extra: ";
      errMsg += meta;
      throw std::runtime_error( errMsg.cstr() );
      }

   return content;
   }
