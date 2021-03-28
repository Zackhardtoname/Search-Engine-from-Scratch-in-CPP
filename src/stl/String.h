#ifndef SCOOBLE_STRING_H
#define SCOOBLE_STRING_H
#pragma once

#include "Vector.h"
#include <iostream>
#include <limits>

namespace scb
   {
class String
   {
   public:
      using Iterator = char *;
      using ConstIterator = const char *;

      static constexpr int npos = std::numeric_limits<int>::max();

      ~String() = default;

      String()
         {
         str.pushBack( '\0' );
         }

      String( const char *cstr )
         {
         while ( *cstr )
            str.pushBack( *( cstr++ ) );
         str.pushBack( *cstr );
         }

      String( const char *begin, const char *end )
         {
         while ( begin != end )
            str.pushBack( *( begin++ ) );
         str.pushBack( '\0' );
         }

      String( const char *buffer, uint len ) : String( buffer, buffer + len )
         {
         }

      String( size_t len, char fill )
         {
         while ( len-- )
            str.pushBack( fill );
         str.pushBack( '\0' );
         }

      String( const String & ) = default;

      String( String && ) = default;

      String &operator=( const String & ) = default;

      String &operator=( String && ) = default;

      [[nodiscard]] size_t size() const noexcept
         {
         return str.size() - 1;
         }

      [[nodiscard]] inline size_t length() const noexcept
         {
         return size();
         }

      [[nodiscard]] char *cstr() const
         {
         return str.begin();
         }

      [[nodiscard]] char *data() const
         {
         return cstr();
         }

      [[nodiscard]] char *begin() const
         {
         return str.begin();
         }

      [[nodiscard]] char *end() const
         {
         return str.end() - 1;
         }

      [[nodiscard]] const char *cbegin() const
         {
         return begin();
         }

      [[nodiscard]] const char *cend() const
         {
         return end();
         }

      [[nodiscard]] int find( const String &other, int start = 0 ) const
         {
         if ( start == npos )
            return npos;

         if ( size() < other.size() + start )
            return npos;

         for ( int pos = start;  pos + other.size() < size() + 1;  ++pos )
            {
            if ( other[ 0 ] == str[ pos ] &&
                 memcmp( str.data() + pos, other.str.data(), other.size() ) ==
                    0 )
               return pos;
            }
         return npos;
         }

      [[nodiscard]] String replace( size_t pos, size_t len, String str2 )
         {
         if ( pos < npos )
            {
            while ( len > 0 )
               {
               str.erase( pos );
               --len;
               if ( str[ pos ] == npos )
                  {
                  break;
                  }
               }
            len = 0;
            for ( auto c : str2 )
               {
               str.insert( pos + len, str2[ len ] );
               ++len;
               }
            }
         return *this;
         }

      [[nodiscard]] int findLast( const String &other, int start = 0 ) const
         {
         start = find( other, start );
         if ( start == npos )
            return npos;

         while ( true )
            {
            int next = find( other, start + 1 );
            if ( next == npos )
               return start;
            start = next;
            }
         }

      [[nodiscard]] bool hasPrefix( const String &other ) const
         {
         if ( size() < other.size() )
            return false;
         for ( int i = 0;  i < other.size();  i++ )
            if ( str[ i ] != other[ i ] )
               return false;
         return true;
         }

      [[nodiscard]] bool hasSuffix( const String &suffix ) const
         {
         if ( size() < suffix.size() )
            return false;
         for ( int i = 0, base = size() - suffix.size();  i < suffix.size();
               i++ )
            if ( str[ base + i ] != suffix[ i ] )
               return false;
         return true;
         }

      char &operator[]( size_t i ) const
         {
         return str[ i ];
         }

      String &operator+=( const String &other )
         {
         // remove null termination
         str.popBack();
         for ( int i = 0;  i < other.str.size();  i++ )
            str.pushBack( other.str[ i ] );

         return *this;
         }

      String operator+( const String &other ) const
         {
         String copy( *this );
         for ( char c : other )
            copy.pushBack( c );

         return copy;
         }

      String operator+( const char *other )
         {
         return ( *this ) + String( other );
         }

      void pushBack( char c )
         {
         str.back() = c;
         str.pushBack( '\0' );
         }

      // should we return the char? I think so.
      // It is exception safe, unlike w/ templated vec
      // it is also neato
      void popBack()
         {
         str.popBack();
         str.back() = '\0';
         }

      void resize( size_t len, char fill = '\0' )
         {
         str.back() = fill;
         str.resize( len + 1, fill );
         str.back() = '\0';
         }

      bool empty() const
         {
         return str.size() == 1;
         }

      void clear()
         {
         str.clear();
         str.pushBack( '\0' );
         }

      [[nodiscard]] String substr( uint start, int len = String::npos ) const
         {
         if ( start >= size() )
            return String();

         auto endIdx =
            len == npos ? size()
                        : std::min( static_cast<ulong>( start + len ), size() );
         return String( data() + start, data() + endIdx );
         }

      bool operator==( const String &other ) const
         {
         return other.str == str;
         }

      bool operator!=( const String &other ) const
         {
         return other.str != str;
         }

      bool operator<( const String &other ) const
         {
         return other.str > str;
         }

      bool operator>( const String &other ) const
         {
         return other.str < str;
         }

      bool operator<=( const String &other ) const
         {
         return other.str >= str;
         }

      bool operator>=( const String &other ) const
         {
         return other.str <= str;
         }

      void to_lower() noexcept
         {
         for ( auto &character : str )
            character = tolower( character );
         }

      [[nodiscard]] char &front() const
         {
         return str[ 0 ];
         }

      [[nodiscard]] char &back() const
         {
         return str[ length() - 1 ];
         }

      [[nodiscard]] String stripped() const
         {
         int start = 0;
         for ( ;  start < length() && isspace( str[ start ] );  start++ )
            ;

         String result = substr( start );
         while ( !result.empty() && isspace( result.back() ) )
            result.popBack();
         return result;
         }

      uint64_t hash() const
         {
         // FNV-1 hash
         uint64_t result = 0xcbf29ce484222325u;
         uint8_t *pos = reinterpret_cast<uint8_t *>( str.data() );
         uint8_t *beyond = reinterpret_cast<uint8_t *>( str.data() + size() );
         while ( pos < beyond )
            {
            result *= 0x00000100000001B3u;
            result ^= *( pos++ );
            }
         return result;
         }

   private:
      Vector<char> str;
   };

/// Reference to a part of a string.
struct StringRef
   {
      String::Iterator Begin, End;

      StringRef() = default;
      StringRef( const String &s ) : Begin( s.begin() ), End( s.end() )
         {
         }

      [[nodiscard]] scb::String Str() const
         {
         return String( Begin, End );
         }

      [[nodiscard]] bool
      EqualsCaseInsensitive( const scb::String &other ) const;

      [[nodiscard]] bool Empty() const
         {
         return Length() == 0;
         }

      [[nodiscard]] unsigned Length() const
         {
         return (unsigned)( End - Begin );
         }

      [[nodiscard]] StringRef Substr( size_t pos, int len = -1 ) const
         {
         if ( len == -1 )
            len = Length();

         auto begin = Begin + pos;
         if ( begin >= End )
            begin = End;

         auto end = begin + len;
         if ( end >= End )
            end = End;

         StringRef rst;
         rst.Begin = begin;
         rst.End = end;
         return rst;
         }

      [[nodiscard]] String::Iterator begin() const
         {
         return Begin;
         }

      [[nodiscard]] String::Iterator end() const
         {
         return End;
         }
   };

std::ostream &operator<<( std::ostream &os, const String &s );

std::istream &operator>>( std::istream &is, String &s );

   }      // namespace scb
#endif // SCOOBLE_STRING_H
