//
// Created by youh on 2/13/21.
//

#include <cassert>
#include <limits>
#include <memory>

#include "wire.h"

Serializer::Serializer( String fmt )
    : format( std::move( fmt ) ), itFormat( format.begin() )
   {
   }

void Serializer::PutInt( int64_t x )
   {
   assert( itFormat != format.end() && "no more fields" );
   switch ( *( itFormat++ ) )
      {
      case 'b':
         buffer.pushBack( x );
         break;
      case 's':
         {
         buffer += "12";
         *reinterpret_cast<int16_t *>( buffer.data() + buffer.size() - 2 ) = x;
         break;
         }
      case 'i':
         {
         buffer += "1234";
         *reinterpret_cast<int32_t *>( buffer.data() + buffer.size() - 4 ) = x;
         break;
         }
      case 'l':
         {
         buffer += "12345678";
         *reinterpret_cast<int64_t *>( buffer.data() + buffer.size() - 8 ) = x;
         break;
         }
      default:
         assert( "only b/a/i/l is allowed" );
      }
   }

void Serializer::PutString( const String &x )
   {
   assert( itFormat != format.end() && "no more fields" );
   switch ( *( itFormat++ ) )
      {
      case 'a':
         assert( x.size() <= std::numeric_limits<uint8_t>::max() &&
                 "string too long" );
         buffer.pushBack( x.size() );
         break;
      case 'A':
         {
         assert( x.size() <= std::numeric_limits<uint16_t>::max() &&
                 "string too long" );
         uint16_t len = x.size();
         buffer += "12";
         *reinterpret_cast<uint16_t *>( buffer.data() + buffer.size() - 2 ) =
            len;
         break;
         }
      default:
         assert( "only a/A is allowed" );
      }
   buffer += x;
   }

String &Serializer::Get()
   {
   assert( itFormat == format.end() && "not all fields filled" );
   return buffer;
   }

Deserializer::Deserializer( String fmt, const String &buf )
    : format( std::move( fmt ) ), buffer( buf ), itFormat( format.begin() ),
      itBuffer( buffer.cbegin() )
   {
   }

String Deserializer::GetString()
   {
   assert( itBuffer < buffer.end() );
   assert( itFormat != format.end() && "no more fields" );
   uint16_t len;
   switch ( *( itFormat++ ) )
      {
      case 'a':
         {
         len = static_cast<uint8_t>( *itBuffer );
         itBuffer++;
         break;
         }
      case 'A':
         {
         len = *reinterpret_cast<const uint16_t *>( &*itBuffer );
         itBuffer += 2;
         break;
         }
      default:
         assert( false && "only a/A is allowed" );
      }
   assert( itBuffer <= buffer.end() );
   auto result = String( itBuffer, itBuffer + len );
   itBuffer += len;
   assert( itBuffer <= buffer.end() );
   return result;
   }

int64_t Deserializer::GetInt()
   {
   assert( itBuffer < buffer.end() );
   assert( itFormat != format.end() && "no more fields" );
   switch ( *( itFormat++ ) )
      {
      case 'b':
         {
         int8_t int8 = *reinterpret_cast<const int8_t *>( &*itBuffer );
         itBuffer++;
         return int8;
         }
      case 's':
         {
         int16_t int16 = *reinterpret_cast<const int16_t *>( &*itBuffer );
         itBuffer += 2;
         return int16;
         }
      case 'i':
         {
         int32_t int32 = *reinterpret_cast<const int32_t *>( &*itBuffer );
         itBuffer += 4;
         return int32;
         }
      case 'l':
         {
         int64_t int64 = *reinterpret_cast<const int64_t *>( &*itBuffer );
         itBuffer += 8;
         return int64;
         }
      default:
         assert( false && "only b/a/i/l is allowed" );
      }
   }

int Deserializer::BytesConsumed()
   {
   return itBuffer - buffer.begin();
   }

void Deserializer::Rearm( String fmt )
   {
   this->format = std::move( fmt );
   itFormat = this->format.begin();
   }

void Deserializer::Rearm()
   {
   itFormat = this->format.begin();
   }

bool Deserializer::Parsable()
   {
   assert( itFormat == format.begin() &&
           "checking parsibility in the middle of parsing" );
   auto it = itBuffer;
   for ( char c : format )
      {
      if ( it >= buffer.end() )
         return false;
      switch ( c )
         {
         case 'b':
            it += 1;
            break;
         case 's':
            it += 2;
            break;
         case 'i':
            it += 4;
            break;
         case 'l':
            it += 8;
            break;
         case 'a':
            it += static_cast<uint8_t>( *it );
            if ( it >= buffer.end() )
               return false;
            it += 1;
            break;
         case 'A':
            if ( it + 1 >= buffer.end() )
               return false;
            it += *reinterpret_cast<const uint16_t *>( &*it );
            it += 2;
            break;
         default:
            assert( false && "only b/a/i/l/a/A is allowed" );
         }
      }
   return it <= buffer.end();
   }
