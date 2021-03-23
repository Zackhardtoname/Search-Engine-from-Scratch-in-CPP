//
// Created by youh on 3/9/21.
//

#ifndef CRAWLER_PARSEDURL_H
#define CRAWLER_PARSEDURL_H

#include "../stl/String.h"

using scb::String;

class ParsedUrl
   {
   public:
      String CompleteUrl;
      String Service, Host, Port, HostPort, Path;

      explicit ParsedUrl( const String &url );

      // Normalize the given path with this URL
      String Normalize( const String &path ) const;
   };

#endif // CRAWLER_PARSEDURL_H
