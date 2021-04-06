//
// Created by youh on 3/10/21.
//

#include "Robotstxt.h"

Robotstxt::Robotstxt( const String &content )
   {
   int starSectionPos = content.findCaseInsensitive( "User-agent: *" );
   // tolerate no space after semicolon.
   if ( starSectionPos == String::npos )
      starSectionPos = content.findCaseInsensitive( "User-agent:*" );
   // tolerate double space after semicolon.
   if ( starSectionPos == String::npos )
      starSectionPos = content.findCaseInsensitive( "User-agent:  *" );
   if ( starSectionPos == String::npos )
      throw std::runtime_error( "no \"User-Agent: *\" section in robots.txt" );

   int nextSection =
      content.findCaseInsensitive( "User-agent:", starSectionPos + 1 );
   String starSection =
      content.substr( starSectionPos, nextSection - starSectionPos );
   starSection.pushBack( '\n' ); // to guarantee that we handle the last line

   String line;
   for ( char c : starSection )
      {
      if ( c == '\r' || c == '\n' )
         {
         // ignore multiple consecutive \r\n
         if ( line.empty() )
            continue;
         handleLine( line );
         line = "";
         }
      else
         line.pushBack( c );
      }

   Sitemap.shrinkToFit();
   rules.shrinkToFit();
   }

void Robotstxt::handleLine( const String &line )
   {
   if ( line.hasPrefix( "Allow:" ) || line.hasPrefix( "Disallow:" ) )
      {
      String url = line.substr( line.find( ":" ) + 1 ).stripped();
      if ( !url.empty() )
         rules.emplaceBack( url, line[ 0 ] == 'A' );
      }
   else if ( line.hasPrefix( "Sitemap: " ) )
      {
      Sitemap = line.substr( line.find( ":" ) + 1 ).stripped();
      }
   // ignore other stuffs
   }

bool Robotstxt::Allow( const String &path )
   {
   int allowLen = 0, disallowLen = -1;
   for ( const CrawlingRule &rule : rules )
      {
      if ( int match = rule.Match( path ) )
         {
         if ( rule.Allowed )
            allowLen = std::max( allowLen, match );
         else
            disallowLen = std::max( disallowLen, match );
         }
      }

   // if equal, the writer probably intend to not allow it
   return allowLen > disallowLen;
   }

CrawlingRule::CrawlingRule( const String &path, bool allowed )
    : Allowed( allowed )
   {
   int it = 0;

   // Prefix
   for ( ;  it < path.length() && path[ it ] != '*' && path[ it ] != '$';
         it++ )
      Prefix.pushBack( path[ it ] );

   // Wildcard
   if ( it < path.length() && path[ it ] == '*' )
      {
      HasWildcard = true;
      it++;
      }

   // Suffix
   // If there are more star symbol, stop right before it
   for ( ;  it < path.length() && path[ it ] != '$' && path[ it ] != '*';
         it++ )
      Suffix.pushBack( path[ it ] );

   // End-of-string
   HasEOS = it < path.length() && path[ it ] == '$';

   Prefix.shrinkToFit();
   Suffix.shrinkToFit();
   }

int CrawlingRule::Match( const String &path ) const
   {
   if ( !path.hasPrefix( Prefix ) )
      return 0;

   if ( HasWildcard )
      {
      if ( HasEOS )
         {
         // /prefix*suffix$
         if ( Prefix.length() + Suffix.length() <= path.length() &&
              path.substr( path.length() - Suffix.length() ) == Suffix )
            return path.length();
         return 0;
         }

      if ( Suffix.empty() )
         return Prefix.length();

      // /prefix*suffix
      int suffixStart = path.findLast( Suffix, Prefix.length() );
      if ( suffixStart == String::npos )
         return 0;
      return suffixStart + Suffix.length();
      }

   // no wildcard
   if ( HasEOS )
      {
      // /prefix$
      if ( path.length() == Prefix.length() )
         return path.length();
      return 0;
      }

   // /prefix
   return Prefix.length();
   }
