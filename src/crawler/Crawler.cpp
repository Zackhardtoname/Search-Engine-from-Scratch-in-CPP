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

   pid_t pid = fork();
   assert( pid != -1 && "fork() failed" );
   if ( pid == 0 )
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
         "/usr/bin/curl",
         "--disable",               // don't read .curlrc
         "--fail",                  // error whenever not HTTP 200 OK
         "--silent",                // no progress bar to mess up stderr
         "--max-filesize", "5m",    // abort on files larger than this
         "--location",              // follow redirections
         "--max-redirs", "10",      // at most 10 times
         "--connect-timeout", "5", // 10 sec connect timeout
         "--max-time", "15",        // 30 sec total timeout
         "--user-agent",
         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
         "like Gecko) Chrome/88.0.4324.190 Safari/537.36 zacklt@umich.edu",
         "--retry", "1", // retry once if transient failure
         "--write-out",
         "%{stderr}%{content_type}\n%{http_code}", // put these to stderr
         url.cstr(), nullptr );
      assert( !"execl() failed" );
      }

   // parent
   // close the write sides of the pipe, so that we are not blocked
   // by living pipe when the child exits
   close( contentFd[ 1 ] );
   close( infoFd[ 1 ] );

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

   scb::String mime;
   while ( true )
      {
      int got = read( infoFd[ 0 ], buf, sizeof( buf ) );
      if ( got == 0 )
         break;
      if ( got == -1 )
         throw std::runtime_error( strerror( errno ) );
      mime += String( buf, got );
      }

   if ( !mime.empty() && !mime.hasPrefixCaseInsensitive( "text" ) )
      throw std::runtime_error( "not a text/* file" );

   int stat;
   while ( waitpid( pid, &stat, 0 ) == -1 )
      {
      if ( errno != EINTR )
         assert( !"waitpid() failed" );
      }

   if ( WEXITSTATUS( stat ) != 0 )
      throw std::runtime_error( "curl error" );

   return content;
   }
