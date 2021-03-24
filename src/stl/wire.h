//
// Created by youh on 2/13/21.
//

#ifndef STL_WIRE_H
#define STL_WIRE_H

#include "../indexer/VLQ.h"
#include "String.h"
#include <cstdint>

using scb::String;
using scb::StringRef;

template <typename T> struct WireType
   {
      static String ToWire( const T &val );

      static T FromWire( StringRef s, size_t *consumed );
   };

template <> struct WireType<String>
   {
      static String ToWire( const String &val )
         {
         // = [len of str][actual char array]
         return VLQ::Encode( val.size() ) + val;
         }

      static String FromWire( StringRef buf, size_t *consumed )
         {
         size_t headerLen;
         uint64_t strLen = VLQ::Decode( buf, &headerLen );
         *consumed = headerLen + strLen;
         return buf.Substr( headerLen, strLen ).Str();
         }
   };

template <typename T> struct WireType<scb::Vector<T>>
   {
      static String ToWire( const scb::Vector<T> &vals )
         {
         // = [num elements][elem1][elem2]...
         String rst = VLQ::Encode( vals.size() );
         for ( const T &val : vals )
            rst += WireType<T>::ToWire( val );
         return rst;
         }

      static scb::Vector<T> FromWire( StringRef buf, size_t *consumed )
         {
         uint64_t numElements = VLQ::Decode( buf, consumed );
         buf = buf.Substr( *consumed );

         scb::Vector<T> rst;
         for ( int i = 0;  i < numElements;  i++ )
            {
            size_t sizeEle;
            rst.pushBack( WireType<T>::FromWire( buf, &sizeEle ) );
            buf = buf.Substr( sizeEle );
            *consumed += sizeEle;
            }

         return rst;
         }
   };

template <typename T> String Serialize( const T &val )
   {
   return WireType<T>::ToWire( val );
   }

template <typename T> T Deserialize( StringRef buf, size_t *consumed )
   {
   return WireType<T>::FromWire( buf, consumed );
   }

#endif // STL_WIRE_H
