//
// Created by youh on 4/25/21.
//

#ifndef SCOOBLE_SEARCH_JSON_H
#define SCOOBLE_SEARCH_JSON_H

#include "../stl/String.h"

// Clean JSON string to be properly displayed
scb::String CleanJsonString( const scb::String &s )
   {
   scb::String escaped;
   for ( char c : s )
      {
      switch ( c )
         {
         case '"':
            escaped += "\\\"";
            break;
         case '\t':
            escaped.pushBack( ' ' );
            break;
         case '\n':
         case '\r':
            break;
         default:
            escaped.pushBack( c );
         }
      }
   return escaped;
   }

#endif // SCOOBLE_SEARCH_JSON_H
