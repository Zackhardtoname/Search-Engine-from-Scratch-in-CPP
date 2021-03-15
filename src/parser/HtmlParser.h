#ifndef SCOOBLE_SEARCH_HTMLPARSER_H
#define SCOOBLE_SEARCH_HTMLPARSER_H

#include <utility>

#include "../stl/include/io/utils.h"
#include "../stl/include/String.h"
#include "../stl/include/Vector.h"

using scb::String;
using scb::StringRef;
using scb::Vector;

class Link {
public:
    String URL;
    Vector<String> anchorText;

    explicit Link(String URL) : URL(std::move(URL)) {}
};

/// A Html Parser
///
/// Parsing:
///
/// Lexing:
///
/// The munch* series functions try to munch the given HTML structure,
/// and return true on success. If failure (return false), the cursor
/// is guaranteed to remain unchanged.
/// For munch* functions with a sink (a output-storage passed in as a
/// reference), the content of these sinks are undefined if munch fails.
/// This is so that the munch* functions don't need to maintain and pass
/// around burdensome inner storage. And if munch fails, these output-storage
/// are not used anyway.
///
/// After the whole document is munched, a list of html entities is stored
/// for later parsing.
/// TODO: after project 3, refactor the lexer out of the parser for easy testing.
class HtmlParser {
public:

    Vector<String> words, titleWords;
    Vector<Link> links;
    String base;

    // The constructor is given a buffer and length containing
    // presumed HTML.  It will parse the buffer, stripping out
    // all the HTML tags and producing the list of words in body,
    // words in title, and links found on the page.
    explicit HtmlParser(const String &buffer);

protected:

    /// *** Parsing ***
    void parse();

    /// Split text by space and store the words into sink.
    static void splitText(StringRef s, Vector<String> &sink);

    /// *** Lexing ***
};

#endif //SCOOBLE_SEARCH_HTMLPARSER_H
