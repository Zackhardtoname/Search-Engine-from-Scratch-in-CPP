//
// Created by youh on 3/15/21.
//

#include "PorterStemming.h"

namespace porter
   {
String label( const String &word )
   {
   // type of letters: consonant or vowel
   String labels( word.length(), 'c' );
   for ( int i = 0;  i < word.length();  i++ )
      switch ( word[ i ] )
         {
         case 'a':
         case 'e':
         case 'i':
         case 'o':
         case 'u':
            labels[ i ] = 'v';
            break;
         case 'y':
            if ( i != 0 && labels[ i - 1 ] == 'c' )
               labels[ i ] = 'v';
            break;
         }
   return labels;
   }

int measure( const String &word )
   {
   String labels = label( word );

   // calculate measure m
   int m = 0;

   // skip Cs in the front
   int i = 0;
   for ( ;  i < word.length() && labels[ i ] == 'c';  i++ )
      ;

   while ( i < word.length() )
      {
      // consume Vs
      for ( ;  i < word.length() && labels[ i ] == 'v';  i++ )
         ;
      if ( i < word.length() )
         {
         m++;
         // consume Cs
         for ( ;  i < word.length() && labels[ i ] == 'c';  i++ )
            ;
         }
      }

   return m;
   }

bool staro( const String &stem )
   {
   if ( stem.length() < 3 )
      return false;
   auto labels = label( stem );
   int i = labels.length() - 3;
   return labels[ i ] == 'c' && labels[ i + 1 ] == 'v' &&
          labels[ i + 2 ] == 'c' &&
          !( stem[ i + 2 ] == 'w' || stem[ i + 2 ] == 'x' ||
             stem[ i + 2 ] == 'y' );
   }

bool suffixMeasure( String &word, const String &suffix, const String &rpl,
                    int m )
   {
   if ( word.hasSuffix( suffix ) )
      {
      String stem = word.substr( 0, word.length() - suffix.length() );
      if ( measure( stem ) > m )
         {
         for ( char c : suffix )
            word.popBack();
         word += rpl;
         }
      // see Porter himself's clarification at
      // https://tartarus.org/martin/PorterStemmer/def.txt
      return true;
      }
   return false;
   }

void step1a( String &word )
   {
   if ( !word.hasSuffix( "s" ) )
      return;
   if ( word.hasSuffix( "sses" ) )
      {
      word.popBack();
      word.popBack();
      return;
      }
   if ( word.hasSuffix( "ies" ) )
      {
      word.popBack();
      word.popBack();
      return;
      }
   if ( word.hasSuffix( "ss" ) )
      return;
   if ( word.hasSuffix( "s" ) )
      {
      word.popBack();
      return;
      }
   }

bool step1b( String &word )
   {
   if ( word.hasSuffix( "d" ) )
      {
      if ( suffixMeasure( word, "eed", "ee" ) )
         return false;
      if ( word.hasSuffix( "ed" ) )
         {
         String stem = word.substr( 0, word.length() - 2 );
         if ( label( stem ).find( "v" ) != String::npos )
            {
            word.popBack();
            word.popBack();
            return true;
            }
         return false;
         }
      }
   if ( word.hasSuffix( "ing" ) )
      {
      String stem = word.substr( 0, word.length() - 3 );
      if ( label( stem ).find( "v" ) != String::npos )
         {
         word.popBack();
         word.popBack();
         word.popBack();
         return true;
         }
      return false;
      }
   return false;
   }

void step1bcont( String &word )
   {
   if ( word.hasSuffix( "at" ) )
      {
      word.pushBack( 'e' );
      return;
      }
   if ( word.hasSuffix( "bl" ) )
      {
      word.pushBack( 'e' );
      return;
      }
   if ( word.hasSuffix( "iz" ) )
      {
      word.pushBack( 'e' );
      return;
      }

   if ( word.length() >= 2 )
      {
      String labels = label( word );
      int i2 = labels.size() - 1;
      int i1 = i2 - 1;
      if ( labels[ i1 ] == 'c' && labels[ i2 ] == 'c' &&
           word[ i1 ] == word[ i2 ] &&
           !( word[ i1 ] == 'l' || word[ i1 ] == 's' || word[ i1 ] == 'z' ) )
         {
         word.popBack();
         return;
         }
      }

   if ( measure( word ) == 1 && staro( word ) )
      word.pushBack( 'e' );
   }

void step1c( String &word )
   {
   if ( word.hasSuffix( "y" ) )
      {
      String stem = word.substr( 0, word.length() - 1 );
      if ( label( stem ).find( "v" ) != String::npos )
         word.back() = 'i';
      }
   }

void step2( String &word )
   {
   if ( word.size() < 2 )
      return;

   switch ( word[ word.length() - 2 ] )
      {
      case 'a':
         if ( suffixMeasure( word, "ational", "ate" ) )
            return;
         if ( suffixMeasure( word, "tional", "tion" ) )
            return;
         break;
      case 'c':
         if ( suffixMeasure( word, "enci", "ence" ) )
            return;
         if ( suffixMeasure( word, "anci", "ance" ) )
            return;
         break;
      case 'e':
         if ( suffixMeasure( word, "izer", "ize" ) )
            return;
         break;
      case 'g':
         if ( suffixMeasure( word, "logi", "log" ) )
            return;
         break;
      case 'l':
         if ( suffixMeasure( word, "bli", "ble" ) )
            return;
         if ( suffixMeasure( word, "alli", "al" ) )
            return;
         if ( suffixMeasure( word, "entli", "ent" ) )
            return;
         if ( suffixMeasure( word, "eli", "e" ) )
            return;
         if ( suffixMeasure( word, "ousli", "ous" ) )
            return;
         break;
      case 'o':
         if ( suffixMeasure( word, "ization", "ize" ) )
            return;
         if ( suffixMeasure( word, "ation", "ate" ) )
            return;
         if ( suffixMeasure( word, "ator", "ate" ) )
            return;
         break;
      case 's':
         if ( suffixMeasure( word, "alism", "al" ) )
            return;
         if ( suffixMeasure( word, "iveness", "ive" ) )
            return;
         if ( suffixMeasure( word, "fulness", "ful" ) )
            return;
         if ( suffixMeasure( word, "ousness", "ous" ) )
            return;
         break;
      case 't':
         if ( suffixMeasure( word, "aliti", "al" ) )
            return;
         if ( suffixMeasure( word, "iviti", "ive" ) )
            return;
         if ( suffixMeasure( word, "biliti", "ble" ) )
            return;
         break;
      }
   }

void step3( String &word )
   {
   if ( word.size() < 2 )
      return;
   switch ( word[ word.length() - 2 ] )
      {
      case 't':
         if ( suffixMeasure( word, "icate", "ic" ) )
            return;
         if ( suffixMeasure( word, "iciti", "ic" ) )
            return;
         break;
      case 'v':
         if ( suffixMeasure( word, "ative", "" ) )
            return;
         break;
      case 'z':
         if ( suffixMeasure( word, "alize", "al" ) )
            return;
         break;
      case 'a':
         if ( suffixMeasure( word, "ical", "ic" ) )
            return;
         break;
      case 'u':
         if ( suffixMeasure( word, "ful", "" ) )
            return;
         break;
      case 's':
         if ( suffixMeasure( word, "ness", "" ) )
            return;
         break;
      }
   }

void step4( String &word )
   {
   if ( word.size() < 2 )
      return;
   switch ( word[ word.length() - 2 ] )
      {
      case 'a':
         if ( suffixMeasure( word, "al", "", 1 ) )
            return;
         break;
      case 'c':
         if ( suffixMeasure( word, "ance", "", 1 ) )
            return;
         if ( suffixMeasure( word, "ence", "", 1 ) )
            return;
         break;
      case 'e':
         if ( suffixMeasure( word, "er", "", 1 ) )
            return;
         break;
      case 'i':
         if ( suffixMeasure( word, "ic", "", 1 ) )
            return;
         break;
      case 'l':
         if ( suffixMeasure( word, "able", "", 1 ) )
            return;
         if ( suffixMeasure( word, "ible", "", 1 ) )
            return;
         break;
      case 'n':
         if ( suffixMeasure( word, "ant", "", 1 ) )
            return;
         if ( suffixMeasure( word, "ement", "", 1 ) )
            return;
         if ( suffixMeasure( word, "ment", "", 1 ) )
            return;
         if ( suffixMeasure( word, "ent", "", 1 ) )
            return;
         break;
      case 'o':
         if ( word.hasSuffix( "ion" ) )
            {
            String stem = word.substr( 0, word.length() - 3 );
            if ( !stem.empty() && ( stem.back() == 's' || stem.back() == 't' ) )
               if ( measure( stem ) > 1 )
                  word = stem;
            return;
            }
         if ( suffixMeasure( word, "ou", "", 1 ) )
            return;
         break;
      case 's':
         if ( suffixMeasure( word, "ism", "", 1 ) )
            return;
         break;
      case 't':
         if ( suffixMeasure( word, "ate", "", 1 ) )
            return;
         if ( suffixMeasure( word, "iti", "", 1 ) )
            return;
         break;
      case 'u':
         if ( suffixMeasure( word, "ous", "", 1 ) )
            return;
         break;
      case 'v':
         if ( suffixMeasure( word, "ive", "", 1 ) )
            return;
         break;
      case 'z':
         if ( suffixMeasure( word, "ize", "", 1 ) )
            return;
         break;
      }
   }

void step5a( String &word )
   {
   if ( word.empty() || word.back() != 'e' )
      return;

   String stem = word.substr( 0, word.length() - 1 );
   int m = measure( stem );

   if ( m > 1 )
      {
      word.popBack();
      return;
      }

   if ( m == 0 )
      return;

   // m == 1
   if ( !staro( stem ) )
      word.popBack();
   }

void step5b( String &word )
   {
   int m = measure( word );
   if ( m <= 1 )
      return;

   if ( word.length() < 2 )
      return;

   String tail = word.substr( word.length() - 2 );

   if ( tail == "ll" )
      {
      word.popBack();
      return;
      }
   }
   } // namespace porter

String PorterStem( const String &input )
   {
   String word;
   // remove 's
   if ( word.hasSuffix( "'s" ) )
      {
      word.popBack();
      word.popBack();
      }

   // convert to lower case and remove non-alphanum characters
   for ( char &c : input )
      if ( isalnum( c ) )
         word.pushBack( tolower( c ) );

   if ( word.length() <= 2 )
      return word;

   // step 1
   porter::step1a( word );
   if ( porter::step1b( word ) )
      porter::step1bcont( word );
   porter::step1c( word );

   porter::step2( word );

   porter::step3( word );

   porter::step4( word );

   porter::step5a( word );
   porter::step5b( word );

   return word;
   }
