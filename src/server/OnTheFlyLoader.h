//
// Created by youh on 4/25/21.
//

#ifndef SCOOBLE_SEARCH_ONTHEFLYLOADER_H
#define SCOOBLE_SEARCH_ONTHEFLYLOADER_H

#include "../parser/ParsedHtml.h"
#include "../stl/String.h"
#include <pthread.h>

struct OnTheFlyResources
   {
      scb::String Title;
      ParsedHtml Html;

      OnTheFlyResources( const scb::String &url, const scb::String &content )
          : Html( content )
         {
         Title = url;
         int titleTagStart = content.find( "<title" );
         if ( titleTagStart == scb::String::npos )
            return;
         int titleTagEnd = content.find( ">", titleTagStart );
         if ( titleTagEnd == scb::String::npos )
            return;
         int titleTextEnd = content.find( "</title", titleTagEnd );
         if ( titleTextEnd == scb::String::npos )
            return;
         int titleTextLen = titleTextEnd - ( titleTagEnd + 1 );
         if ( titleTextLen == 0 )
            return;
         Title = content.substr( titleTagEnd + 1, titleTextLen );
         }
   };

pthread_t SpawnFetchHtml( const scb::String & );

#endif // SCOOBLE_SEARCH_ONTHEFLYLOADER_H
