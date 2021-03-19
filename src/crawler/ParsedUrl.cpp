//
// Created by youh on 3/9/21.
//

#include "ParsedUrl.h"
#include <cstring>

ParsedUrl::ParsedUrl( const String &url )
   {
   // anything after pound is meaningless as page URL
   int pound = url.find( "#" );
   if ( pound == String::npos )
      CompleteUrl = url;
   else
      CompleteUrl = url.substr( 0, pound );

   // Service
   int i = 0;
   for ( ;  i < CompleteUrl.size() && CompleteUrl[ i ] != ':';  i++ )
      ;
   Service = CompleteUrl.substr( 0, i );

   // Host
   i += 3;
   int hostStart = i;
   for ( ;  i < CompleteUrl.size() && CompleteUrl[ i ] != ':' &&
            CompleteUrl[ i ] != '/';
         i++ )
      ;
   Host = CompleteUrl.substr( hostStart, i - hostStart );

   // Port
   if ( i < CompleteUrl.size() && CompleteUrl[ i ] == ':' )
      {
      int portStart = i + 1;
      for ( ;  i < CompleteUrl.size() && CompleteUrl[ i ] != '/';  i++ )
         ;
      Port = CompleteUrl.substr( portStart, i - portStart );
      }

   HostPort = CompleteUrl.substr( hostStart, i - hostStart );

   // Path
   hostStart = i;
   for ( ;  i < CompleteUrl.size() && CompleteUrl[ i ] != '?';  i++ )
      ;
   Path = CompleteUrl.substr( hostStart, i - hostStart );
   if ( Path.empty() )
      Path = "/";
   }

String ParsedUrl::Normalize( const String &path )
   {
   if ( path.hasPrefix( "mailto:" ) || path.hasPrefix( "tel:" ) )
      return ""; // not crawlable
   // edge case
   if ( path.empty() )
      return Service + "://" + HostPort;
   // if fully qualified
   if ( path.hasPrefix( "http://" ) || path.hasPrefix( "https://" ) )
      return ParsedUrl( path ).CompleteUrl;

   // if relative path, we only handle this case and not
   // those nefarious "../" kinda stuffs
   if ( path.hasPrefix( "./" ) )
      {
      String result = Service + "://" + HostPort + Path;
      if ( result.back() != '/' )
         result.pushBack( '/' );
      result += path.substr( 2 );
      return ParsedUrl( result ).CompleteUrl;
      }
   else
      {
      // absolute path
      if ( path.front() == '/' )
         return ParsedUrl( Service + "://" + HostPort + path ).CompleteUrl;
      else
         return ParsedUrl( Service + "://" + HostPort + "/" + path )
            .CompleteUrl;
      }
   }
