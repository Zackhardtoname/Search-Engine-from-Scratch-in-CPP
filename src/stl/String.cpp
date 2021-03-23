//
// Created by youh on 3/9/21.
//

#include "String.h"

using scb::String;
using scb::StringRef;

String StringRef::Str() const
   {
   return String( Begin, End );
   }

bool StringRef::EqualsCaseInsensitive( const scb::String &other ) const
   {
   if ( other.length() != Length() )
      return false;
   String::Iterator i = Begin;
   for ( char c : other )
      {
      if ( tolower( *i ) != tolower( c ) )
         return false;
      i++;
      }
   return true;
   }

bool StringRef::Empty() const
   {
   return Length() == 0;
   }

unsigned StringRef::Length() const
   {
   return (unsigned)( End - Begin );
   }

std::ostream &scb::operator<<( std::ostream &os, const scb::String &s )
   {
   for ( auto character : s )
      os << character;
   return os;
   }

std::istream &scb::operator>>( std::istream &is, String &s )
   {
   s.clear();

   while ( is )
      {
      char p;
      is >> p;
      if ( isspace( p ) )
         return is;
      s.pushBack( p );
      }
   return is;
   }
