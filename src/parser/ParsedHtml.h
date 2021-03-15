#ifndef SCOOBLE_SEARCH_PARSEDHTML_H
#define SCOOBLE_SEARCH_PARSEDHTML_H

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
    Vector<String> AnchorText;

    explicit Link(String URL) : URL(std::move(URL)) {}
};

/// A Html Parser
class ParsedHtml {
public:

    Vector<String> Words, TitleWords;
    Vector<Link> Links;
    String Base;
    String Language;

    // The constructor is given a String containing
    // presumed HTML. It will parse the buffer, stripping out
    // all the HTML tags and producing the list of words in body,
    // words in title, and links found on the page.
    explicit ParsedHtml(const String &buffer);

protected:
    /// Split text by space and store the words into sink.
    static void splitText(StringRef s, Vector<String> &sink);
};

#endif //SCOOBLE_SEARCH_PARSEDHTML_H
