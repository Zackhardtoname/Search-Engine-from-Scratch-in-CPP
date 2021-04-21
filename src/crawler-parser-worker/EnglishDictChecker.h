//
// Created by youh on 3/15/21.
//

#ifndef SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H
#define SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H

#include <cassert>
#include <fstream>
#include <string>
#include <unordered_set>

#include "../indexer/IndexChunk.h"
#include "../indexer/PorterStemming.h"
#include "../parser/ParsedHtml.h"
#include "../stl/String.h"

// TODO remove
#include "../stl/StringHash.h"

using scb::String;
using scb::StringHash;

/// Check if a given HTML is in English by comparing the words
/// with those in a given dictionary. The dictionary should
/// be stemmed, and the HTML passed in is assumed to have been
/// stemmed.

class EnglishDictChecker
   {
      constexpr static float ENGLISH_THRESHOLD = 0.35;

      // TODO: change this to our own unordered_set once we have one
      std::unordered_set<String, StringHash> vocab;

   public:
      // Check if the given HTML is in English. Assuming
      // the words in the document have been stemmed.
      bool Check( const ParsedHtml &parsedHtml )
         {
         int hit = 0;
         for ( String &word : parsedHtml.Words )
            if ( vocab.count( word ) )
               hit++;

         // it is in English if at least this proportion of words are English
         return hit > parsedHtml.Words.size() * ENGLISH_THRESHOLD;
         }

      explicit EnglishDictChecker( const char *vocabFile )
         {
         std::fstream f( vocabFile );
         if ( !f )
            throw std::runtime_error( "cannot open dictionary file" );
         // read in pre-stemmed words from the dictionary
         scb::String word;
         while ( f >> word )
            vocab.insert( word );
         assert( !vocab.empty() && "empty vocabulary" );
         }
   };

#endif // SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H
