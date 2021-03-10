//
// Created by youh on 2/13/21.
//

#ifndef STL_WIRE_H
#define STL_WIRE_H

#include "String.h"
#include <cstdint>

using agr::String;

// Format:
// %b: a signed byte
// %s: short, 16bit signed integer
// %i: int, 32bit signed integer
// %l: long, 64bit signed integer
// %a: array of char, maximal length 256
// %A: array of char, maximal length 65536
class Serializer
   {
      String format, buffer;
      String::Iterator itFormat;

   public:
      explicit Serializer( String fmt );

      void PutInt( int64_t x );

      void PutString( const String &x );

      String &Get();
   };

class Deserializer
   {
      String format;
      const String &buffer;
      String::Iterator itFormat;
      String::ConstIterator itBuffer;

   public:
      Deserializer( String fmt, const String &buf );

      int64_t GetInt();

      String GetString();

      void Rearm( String fmt );

      void Rearm();

      int BytesConsumed();

      bool Parsable();
   };

#endif // STL_WIRE_H
