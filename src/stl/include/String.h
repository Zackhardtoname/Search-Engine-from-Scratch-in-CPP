#ifndef SCOOBLE_STRING_H
#define SCOOBLE_STRING_H
#pragma once

#include "Vector.h"
#include <iostream>
namespace agr
   {
class String
   {
   public:
      using Iterator = char *;
      using ConstIterator = const char *;

      static constexpr size_t npos = -1;

      ~String()
         {
         str.clear();
         }

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

    String( const char *buffer, uint len ): String(buffer, buffer+len)
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

      [[nodiscard]] const int find( String &other, int start = 0 ) const
         {
         if ( size() >= other.size() )
            {
            int sz = size();
            bool found = true;
            for ( int pos = start;  pos < sz;  ++pos )
               {
               if ( other[ 0 ] == str[ pos ] )
                  {
                  for ( auto character : other )
                     {
                     if ( character != str[ pos ] )
                        found = false;
                     ++pos;
                     }
                  if ( found )
                     return pos - other.size();
                  }
               }
            return sz + 1;
            }
         return size() + 1;
         }
      [[nodiscard]] const int find( const char *&other, int start = 0 ) const
         {
         if ( size() >= strlen( other ) )
            {
            int sz = size();
            int osz = strlen( other );
            bool found = true;
            for ( int pos = start;  pos < sz;  ++pos )
               {
               if ( *other == str[ pos ] )
                  {
                  while ( *other != '\0' )
                     {
                     if ( pos >= sz || *other != str[ pos ] )
                        found = false;
                     ++pos, ++other;
                     }
                  if ( found )
                     return pos - osz;
                  }
               }
            return sz + 1;
            }
         return size() + 1;
         }

      [[nodiscard]] const int find( const char *other, int start = 0 ) const
         {
         if ( size() >= strlen( other ) )
            {
            int sz = size();
            int osz = strlen( other );
            bool found = true;
            for ( int pos = start;  pos < sz;  ++pos )
               {
               if ( *other == str[ pos ] )
                  {
                  while ( *other != '\0' )
                     {
                     if ( pos >= sz || *other != str[ pos ] )
                        found = false;
                     ++pos, ++other;
                     }
                  if ( found )
                     return pos - osz;
                  }
               }
            return sz + 1;
            }
         return size() + 1;
         }

      char &operator[]( size_t i )
         {
         return str[ i ];
         }

      void operator+=( const String &other )
         {
         // remove null termination
         if ( !str.empty() )
            str.popBack();
         // add other string to it. stops problem
         // of the mondel burrito i think its called
         for ( int i = 0;  i < other.size() + 1;  i++ )
            str.pushBack( other.str[ i ] );
         }

      String operator+( const String &other )
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
         if ( !str.empty() )
            str.popBack();
         str.pushBack( c );
         str.pushBack( '\0' );
         }

      // should we return the char? I think so.
      // It is exception safe, unlike w/ templated vec
      // it is also neato
      void popBack()
         {
         str.popBack();
         str.popBack();
         str.pushBack( '\0' );
         }

      void resize( size_t len, char fill = '\0' )
         {
         str.back() = fill;
         str.resize( len + 1, fill );
         str.back() = '\0';
         }

      bool empty()
         {
         return str.size() == 1;
         }

      [[nodiscard]] String substr( uint start, size_t len = String::npos ) const
         {
         auto endIdx = len == npos ? size() : std::min( start + len, size() );
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

   private:
      Vector<char> str;
   };

std::ostream &operator<<( std::ostream &os, const String &s );

   }      // namespace agr
#endif // SCOOBLE_STRING_H
