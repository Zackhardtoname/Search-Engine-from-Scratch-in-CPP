#ifndef SCOOBLE_SEARCH_PARSEDHTML_H
#define SCOOBLE_SEARCH_PARSEDHTML_H

#include <utility>

#include "../stl/String.h"
#include "../stl/Vector.h"
#include "../stl/utils.h"

using scb::String;
using scb::StringRef;
using scb::Vector;

/// A Html Parser
class ParsedHtml
   {
   public:
      Vector<String> Words, TitleWords;
      Vector<String> Links;
      String Language;

      // The constructor is given a String containing
      // presumed HTML. It will parse the buffer, stripping out
      // all the HTML tags and producing the list of words in body,
      // words in title, and links found on the page.
      explicit ParsedHtml( const String &buffer );

   protected:
      void addWord( String &word );

      /// Split text by space and store the words into sink.
      static void splitText( StringRef s, Vector<String> &sink );
   };

#endif // SCOOBLE_SEARCH_PARSEDHTML_H
