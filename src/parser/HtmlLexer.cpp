//
// Created by youh on 3/14/21.
//

#include <cassert>

#include "HtmlLexer.h"

HtmlLexer::HtmlLexer( const scb::String &buffer )
    : cursor( buffer.cstr() ), bufEnd( buffer.end() )
   {
   }

int HtmlLexer::peek( const unsigned offset )
   {
   if ( cursor + offset >= bufEnd )
      return -1;
   return *( cursor + offset );
   }

bool HtmlLexer::peekAssert( const scb::String &s )
   {
   for ( unsigned i = 0;  i < s.length();  i++ )
      if ( peek( i ) != s[ i ] )
         return false;
   return true;
   }

bool HtmlLexer::HasNextEntity() const
   {
   return !eof();
   }

HtmlEntity &HtmlLexer::NextEntity()
   {
   if ( eof() )
      throw std::range_error( "entity exhausted" );
   curEntity.Content.Begin = cursor; // this entity starts right here
   bool success = munchComment( true ) || munchRawTextElement() ||
                  munchStartTag() || munchEndTag() || munchSelfEndingTag() ||
                  munchTextAtLeastOneChar();
   assert( success && "unhandled case" );
   curEntity.Content.End = cursor; // shouldn't consume more than we munched
   return curEntity;
   }

bool HtmlLexer::munchTextAtLeastOneChar()
   {
   curEntity.Type = HtmlEntity::Text;
   if ( eof() )
      return false;

   // munch at least one char, because previous tag munching all failed
   do
      {
      munchAny();
      }
   while ( !eof() && !peekAssert( "<" ) );

   return true;
   }

bool HtmlLexer::munchChar( const char c )
   {
   if ( peek() != c )
      return false;
   cursor++;
   return true;
   }

bool HtmlLexer::munchAny()
   {
   if ( eof() )
      return false;
   cursor++;
   return true;
   }

bool HtmlLexer::munchSpace()
   {
   if ( !isspace( peek() ) )
      return false;
   munchAny();
   return true;
   }

bool HtmlLexer::munchRawTextElement()
   {
   curEntity.Type = HtmlEntity::RawTextElement;

   auto savepoint = checkpoint();
   curEntity.TagName.Begin = cursor + 1; // skip <
   // the five raw text elems we need to handle
   // svg is technically not, but we don't care what's inside
   if ( munchStringCaseInsensitive( "<script" ) ||
        munchStringCaseInsensitive( "<style" ) ||
        munchStringCaseInsensitive( "<textarea" ) ||
        munchStringCaseInsensitive( "<svg" ) ||
        munchStringCaseInsensitive( "<title" ) )
      curEntity.TagName.End = cursor;
   else
      return false;

   curEntity.Attributes.clear();
   // ele = <TagName + [  attr >] + [raw text] + [end tag with closing name]
   if ( munchTagAfterTagName( curEntity.Attributes ) &&
        munchRawText( curEntity.InnerText ) &&
        munchEndTag( curEntity.TagName ) )
      {
      savepoint.Commit();
      return true;
      }
   return false;
   }

bool HtmlLexer::munchComment( bool treatAsEntity )
   {
   auto savepoint = checkpoint();
   // a comment starts with "<!-- ", note the space.
   if ( !munchStringCaseInsensitive( "<!--" ) )
      return false;
   if ( !isspace( peek() ) )
      return false;

   while ( !eof() )
      {
      // close with " -->", note the space
         {
         auto closingSave = checkpoint();
         if ( munchSpace() && munchStringCaseInsensitive( "-->" ) )
            {
            closingSave.Commit();
            savepoint.Commit();
            if ( treatAsEntity )
               curEntity.Type = HtmlEntity::Comment;
            return true;
            }
         }

      // otherwise, take in any char
      munchAny();
      }
   return false;
   }

bool HtmlLexer::munchTagAfterTagName( Vector<Attribute> &attributes,
                                      bool selfEnding )
   {
   scb::String endSeq = selfEnding ? "/>" : ">";
   // [x] must be either endSeq or space in
   // <TagName[x]...
   if ( !peekAssert( endSeq ) && !isspace( peek() ) )
      return false;

   auto savepoint = checkpoint();
   while ( !eof() && !peekAssert( endSeq ) )
      {
      // munch away all spaces before the next attribute or endSeq
      while ( munchSpace() )
         ;

      // if eof, munching failed
      if ( eof() )
         return false;

      // if neither endSeq nor an attribute follows, munching failed
      if ( peekAssert( endSeq ) )
         continue;

      Attribute attr;
      if ( munchAttribute( attr ) )
         {
         attributes.pushBack( attr );
         continue;
         }

      return false;
      }
   // now that either eof, or we have found endSeq
   if ( eof() )
      return false;
   assert( munchStringCaseInsensitive( endSeq ) );
   savepoint.Commit();
   return true;
   }

bool HtmlLexer::munchRawText( StringRef &s )
   {
   auto savepoint = checkpoint();
   s.Begin = cursor;
   while ( !eof() )
      {
      // According to w3, raw text can contain comment tag.
      if ( munchComment() )
         continue;

      // if followed by </, we're done with raw text.
      if ( peekAssert( "</" ) )
         {
         savepoint.Commit();
         s.End = cursor;
         return true;
         }

      // otherwise, take whatever char in
      munchAny();
      }

   // eof, probably a malformed closing tag or something.
   // should give other choice a chance.
   return false;
   }

bool HtmlLexer::munchStartTag()
   {
   curEntity.Type = HtmlEntity::StartTag;

   auto savepoint = checkpoint();
   curEntity.Attributes.clear();
   // start tag = < + tagName + [ attr1 attr2=v >]
   if ( munchChar( '<' ) && munchTagName( curEntity.TagName ) &&
        munchTagAfterTagName( curEntity.Attributes ) )
      {
      savepoint.Commit();
      return true;
      }
   return false;
   }

bool HtmlLexer::munchEndTag()
   {
   curEntity.Type = HtmlEntity::EndTag;

   auto savepoint = checkpoint();
   // end tag = </ + tagName + [   >]
   curEntity.Attributes
      .clear(); // it's technically illegal, but there's no harm
   if ( munchStringCaseInsensitive( "</" ) &&
        munchTagName( curEntity.TagName ) &&
        munchTagAfterTagName( curEntity.Attributes ) )
      {
      savepoint.Commit();
      return true;
      }
   return false;
   }

bool HtmlLexer::munchEndTag( const StringRef &tagName )
   {
   auto savepoint = checkpoint();
   Vector<Attribute> attributes; // dummy
   // end tag = </ + tagName + [   >]
   if ( munchStringCaseInsensitive( "</" ) &&
        munchStringCaseInsensitive( tagName ) &&
        munchTagAfterTagName( attributes ) )
      {
      savepoint.Commit();
      return true;
      }
   return false;
   }

bool HtmlLexer::munchSelfEndingTag()
   {
   curEntity.Type = HtmlEntity::SelfEndingTag;

   auto savepoint = checkpoint();
   curEntity.Attributes.clear();
   // a self ending tag starts with <TagName
   if ( munchStringCaseInsensitive( "<" ) &&
        munchTagName( curEntity.TagName ) &&
        munchTagAfterTagName( curEntity.Attributes, true ) )
      {
      savepoint.Commit();
      return true;
      }
   return false;
   }

bool HtmlLexer::munchStringCaseInsensitive( const scb::String &s )
   {
   auto savepoint = checkpoint();
   // compare each char in string to next peeked char
   // if equal, munch
   // else fail
   for ( char c : s )
      {
      if ( tolower( c ) != tolower( peek() ) )
         return false;
      munchAny();
      }
   savepoint.Commit();
   return true;
   }

bool HtmlLexer::munchStringCaseInsensitive( const StringRef &s )
   {
   auto savepoint = checkpoint();
   // compare each char in string to next peeked char
   // if equal, munch
   // else fail
   for ( auto i = s.Begin;  i != s.End;  i++ )
      {
      if ( tolower( *i ) != tolower( peek() ) )
         return false;
      munchAny();
      }
   savepoint.Commit();
   return true;
   }

bool HtmlLexer::munchTagName( StringRef &tagNameSink )
   {
   auto savepoint = checkpoint();
   // a tag name is a non-empty string, ending with space, /, or >
   tagNameSink.Begin = cursor;
   bool munchedSomething = false;
   while ( !eof() )
      {
      char c = (char)peek();
      if ( isspace( c ) || c == '/' || c == '>' )
         break;
      munchAny();
      munchedSomething = true;
      }
   // only succeeds when we munched at least one char
   if ( munchedSomething )
      {
      savepoint.Commit();
      tagNameSink.End = cursor;
      return true;
      }
   return false;
   }

bool HtmlLexer::eof() const
   {
   return cursor >= bufEnd;
   }

bool HtmlLexer::munchAttribute( Attribute &attr )
   {
   auto savepoint = checkpoint();
   // an attribute is either "attr", or "attr  =  value"
   // some non-standard may also quote the key

   // assume no value first
   attr.second.Begin = attr.second.End = cursor;

   attr.first.Begin = cursor;

   // test for non-standard key first
   if ( munchQuotedString( '"' ) || munchQuotedString( '\'' ) )
      {
      attr.first.End = cursor;
      }
   else
      {
      // standard-conforming key
      // munch "attr", until / or > or = or space
      bool munchedSomething = false;
      while ( !eof() )
         {
         char c = (char)peek();
         if ( isspace( c ) || c == '/' || c == '>' || c == '=' )
            break;
         munchAny();
         munchedSomething = true;
         }
      if ( !munchedSomething )
         return false;
      attr.first.End = cursor;
      }

   // we got a key at least. Checkpoint in case we want to come back
   auto munchedKeySave = checkpoint();
   while ( munchSpace() )
      ;

   // if eof after munching all whitespace, this is not our problem anymore.
   // Assuming that this is an attribute without value.
   if ( eof() )
      {
      savepoint.Commit();
      // going back to munchedKeySave
      return true;
      }

   // if anything other than = follows, this is an attribute without value
   if ( peek() != '=' )
      {
      savepoint.Commit();
      return true;
      }

   // followed by =, this attribute has value
   assert( munchChar( '=' ) );
   // ignorance is bliss. We can't pretend that there may be no value anymore.
   munchedKeySave.Commit();

   // munching away all whitespaces. If eof, fail because we expect a value.
   while ( munchSpace() )
      ;
   if ( eof() )
      return false;

   // if it's a nice quoted string
   char c = (char)peek();
   if ( c == '\'' || c == '"' )
      {
      attr.second.Begin = cursor + 1; // discard the quotation mark
      if ( !munchQuotedString( c ) )
         return false;
      savepoint.Commit();
      attr.second.End = cursor - 1; // ditto
      return true;
      }

   // else, it is an ugly unquoted value, try to do our best here
   attr.second.Begin = cursor;
   bool munchedSomething = false;
   while ( !eof() )
      {
      char c = (char)peek();
      if ( isspace( c ) || c == '"' || c == '\'' || c == '=' || c == '>' ||
           c == '/' )
         break;
      munchAny();
      munchedSomething = true;
      }
   if ( !munchedSomething )
      return false;

   savepoint.Commit();
   attr.second.End = cursor;
   return true;
   }

bool HtmlLexer::munchQuotedString( const char mark )
   {
   // a quoted string starts with quotation mark
   // [mark] a b c d [mark]
   if ( peek() != mark )
      return false;
   auto savepoint = checkpoint();
   munchChar( mark ); // munch the opening mark
   while ( !eof() )
      {
      // if followed by a [mark], we're done with the string
      if ( munchChar( mark ) )
         {
         savepoint.Commit();
         return true;
         }
      // otherwise, take whatever char in
      munchAny();
      }
   return false;
   }

HtmlLexer::Savepoint HtmlLexer::checkpoint()
   {
   return HtmlLexer::Savepoint( *this, cursor );
   }

void HtmlLexer::Savepoint::Commit()
   {
   commited = true;
   }

HtmlLexer::Savepoint::Savepoint( HtmlLexer &lexer, String::Iterator save )
    : lexer( lexer ), save( save )
   {
   }

HtmlLexer::Savepoint::~Savepoint()
   {
   if ( !commited )
      lexer.cursor = save;
   }

const Attribute *HtmlEntity::FindAttr( const String &attrName )
   {
   for ( const auto &attr : Attributes )
      if ( attr.first.EqualsCaseInsensitive( attrName ) )
         return &attr;
   return nullptr;
   }
