// HtmlParser.cpp
// Nicole Hamlton, nham@umich.edu

#include "../stl/String.h"
#include "../stl/utils.h"
#include <cassert>
#include <csignal>
#include <iostream>
#include <unordered_map>

#include "ParsedHtml.h"

enum State
{
   Text,
   ExpectTagName,
   WaitTagClose
};

void ParsedHtml::addWord( String &word )
   {
   if ( !word.empty() )
      Words.pushBack( std::move( word ) );
   word.clear();
   }

ParsedHtml::ParsedHtml( const String &buffer )
   {
   String word, tagName;
   State state = Text;

   // find words
   for ( auto cur = buffer.begin(), to = buffer.end();  cur != to;  cur++ )
      {
      char c = *cur;

      switch ( state )
         {
         case Text:
            if ( isspace( c ) || c == '<' )
               {
               addWord( word );
               if ( c == '<' )
                  state = ExpectTagName;
               }
            else
               word.pushBack( c );

            break;

         case ExpectTagName:
            if ( isalnum( c ) )
               {
               tagName.pushBack( c );
               break;
               }
            else
               {
               // complete tag name found, see if it's something we want to
               // ignore that is, <script>, <style>, <svg>
               bool ignore = false;
               if ( tagName.length() > 2 )
                  {
                  switch ( tagName[ 1 ] )
                     {
                     case 'c':
                        if ( tagName == "script" )
                           ignore = true;
                        break;
                     case 't':
                        if ( tagName == "style" )
                           ignore = true;
                        break;
                     case 'v':
                        if ( tagName == "svg" )
                           ignore = true;
                        break;
                     }
                  }

               if ( ignore )
                  {
                  // jump ahead to closing tag </ tagName
                  auto target = String( "</" ) + tagName;
                  tagName.clear();
                  int pos = buffer.find( target, cur - buffer.begin() );
                  if ( pos == String::npos )
                     {
                     // if not found, try to recover by switching into text mode
                     state = Text;
                     break;
                     }

                  cur =
                     buffer.begin() + pos + target.size() - 1; // -1 because ++
                  state = WaitTagClose;
                  break;
                  }

               tagName.clear();

               if ( c == '>' )
                  state = Text;
               else
                  state = WaitTagClose;
               }
            break;

         case WaitTagClose:
            if ( c == '>' )
               state = Text;

            break;
         }
      }

   // find title words
   do
      {
      int titlePos = buffer.find( "<title" );
      if ( titlePos == String::npos )
         break;

      int titleOpenEnd = buffer.find( ">", titlePos );
      if ( titleOpenEnd == String::npos )
         break;

      int titleEnd = buffer.find( "</title", titleOpenEnd );
      if ( titleEnd == String::npos )
         break;

      StringRef title( buffer.begin() + titleOpenEnd + 1,
                       buffer.begin() + titleEnd );
      splitText( title, TitleWords );
      }
   while ( 0 );

   // find links
   int remaining = 0;
   String anchor = "<a";
   String href = "href=\"";
   while ( true )
      {
      int anchorStart = buffer.find( anchor, remaining );
      if ( anchorStart == String::npos )
         break;

      int anchorEnd = buffer.find( ">", anchorStart );
      if ( anchorEnd == String::npos )
         break;

      int hrefPos = buffer.find( href, anchorStart );
      if ( hrefPos == String::npos )
         break;
      if ( hrefPos > anchorEnd )
         {
         remaining = anchorEnd;
         continue;
         }

      int linkStart = hrefPos + href.size();

      int linkEnd = buffer.find( "\"", linkStart );
      if ( linkEnd == String::npos )
         break;

      Links.emplaceBack( buffer.substr( linkStart, linkEnd - linkStart ) );
      remaining = linkEnd;
      }

   // find language
   String lang = "lang=\"";
   int langPos = buffer.find( lang );
   Language = buffer.substr( langPos + lang.size(), 2 );
   }

void ParsedHtml::splitText( StringRef s, Vector<String> &sink )
   {
   String buf;

   for ( String::Iterator i = s.Begin;  i != s.End;  i++ )
      {
      char c = *i;
      if ( !isalnum( c ) )
         {
         if ( !buf.empty() )
            {
            sink.pushBack( std::move( buf ) );
            buf.clear();
            }
         }
      else
         buf.pushBack( c );
      }

   if ( !buf.empty() )
      sink.pushBack( std::move( buf ) );
   }
