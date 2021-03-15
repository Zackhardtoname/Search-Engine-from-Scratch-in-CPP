// HtmlParser.h
// Nicole Hamilton, nham@umich.edu

#pragma once

#include <vector>
#include <list>
#include "HtmlTags.h"
#include "../stl/include/io/utils.h"
#include "../stl/include/String.h"

// This is a simple HTML parser class.  Given a text buffer containing
// a presumed HTML page, the constructor will parse the text to create
// lists of words, title words and outgoing links found on the page.  It
// does not attempt to parse the entire the document structure.
//
// The strategy is to word-break at whitespace and HTML tags and discard
// most HTML tags.  Three tags require discarding everything between
// the opening and closing tag. Four tags require special processing.
//
// We will use the list of possible HTML element names found at
// https://developer.mozilla.org/en-US/docs/Web/HTML/Element +
// !-- (comment), !DOCTYPE and svg, stored as a table in HtmlTags.h.

// Here are the rules for recognizing HTML tags.
//
// 1. An HTML tag starts with either < if it's an opening tag or </ if
//    it's closing token.  If it starts with < and ends with /> it is both.
//
// 2. The name of the tag must follow the < or </ immediately.  There can't
//    be any whitespace.
//
// 3. The name is terminated by whitespace, > or / and is case-insensitive.
//
// 4. If it is terminated by whitepace, arbitrary text representing various
//    arguments may follow, terminated by a > or />.
//
// 5. If the name isn't on the list we recognize, we assume it's the whole
//    is just ordinary text.
//
// 6. Every token is taken as a word-break.
//
// 7. Most opening or closing tokens can simply be discarded.
//
// 8. <script>, <style>, and <svg> require discarding everything between the
//    opening and closing tag.  Unmatched closing tags are discarded.
//
// 9. <title>, <a>, <base> and <embed> require special processing.
//
//      <title> should cause all the words between the opening and closing
//          tags to be added to the titleWords vector rather than the default
//          words vector.  A closing </title> without an opening <title> is discarded.
//
//      <a> is expected to contain an href="...url..."> argument with the
//          URL inside the double quotes that should be added to the list
//          of links.  All the words in between the opening and closing tags
//          should be collected as anchor text associated with the link
//          in addition to being added to the words or titleWords vector,
//          as appropriate.  A closing </a> without an opening <a> is
//          discarded.
//
//     <base> may contain an href="...url..." parameter.  If present, it should
//          be captured as the base variable.  Only the first is recognized; any
//          others are discarded.
//
//     <embed> may contain a src="...url..." parameter.  If present, it should be
//          added to the links with no anchor text.



class Link {
public:
    scb::String URL;
    std::vector<scb::String> anchorText;

    Link(scb::String URL) : URL(URL) {
    }
};


/// Cursor pointing to a character in a buffer.
using Cursor = const char*;


/// Reference to a string in a buffer.
struct StringRef {
    Cursor Begin, End;

    [[nodiscard]] scb::String Str() const;

    [[nodiscard]] bool EqualsCaseInsensitive(const scb::String &other) const;

    [[nodiscard]] bool Empty() const;

    [[nodiscard]] unsigned Length() const;
};


using Attribute = std::pair<StringRef, StringRef>;
struct HtmlEntity {
    enum Type {
        RawTextElement,
        StartTag,
        EndTag,
        SelfEndingTag,
        Comment,
        Text
    } Type;
    /// Before parsed into a structured entity, I was this flat string
    StringRef Content;
    StringRef TagName;
    /// Used by RawTextElement only. The text I contained.
    StringRef InnerText;
    std::vector<Attribute> Attributes;
};


/// A Html Lexer
class HtmlLexer {
public:

    explicit HtmlLexer(const scb::String &buffer);

    [[nodiscard]] bool HasNextEntity() const;

    HtmlEntity& NextEntity();

protected:
    /// Save the state of the lexer. State will be rollback on destructor if
    /// Commit() is not called.
    class Savepoint {
        bool commited = false;
        HtmlLexer &lexer;
        Cursor save;
    public:

        /// Discard the saved state.
        void Commit();

        Savepoint(HtmlLexer &lexer, Cursor save);

        ~Savepoint();
    };

    Cursor cursor, bufEnd;

    HtmlEntity curEntity;

    /// Peek the char under the cursor. Returns -1 if eof.
    int peek(unsigned offset = 0);

    /// Peek and determine if the given string matches the one starting under the cursor.
    bool peekAssert(const scb::String &s);

    /// Munch one of the six raw text elements as defined by W3.
    bool munchRawTextElement();

    /// Munch a <!-- ... --> comment tag.
    /// treatAsEntity: if the munched comment tag should be treat as an entity.
    bool munchComment(bool treatAsEntity = false);

    /// Munch a start tag.
    bool munchStartTag();

    /// Munch an end tag.
    bool munchEndTag();

    /// Munch an end tag that must use the given tag name.
    /// tagName: mandatory tag name of the end tag, case insensitive.
    bool munchEndTag(StringRef tagName);

    /// Munch a self-ending tag.
    bool munchSelfEndingTag();

    /// Munch a paragraph of ordinary text. At least munch one character.
    bool munchTextAtLeastOneChar();

    /// Munch a paragraph of raw text in a raw text element.
    /// s: storage for munched raw text.
    bool munchRawText(StringRef &s);

    /// Munch a given string, case-insensitively.
    bool munchStringCaseInsensitive(const scb::String &s);

    /// Munch a given string, case-insensitively.
    bool munchStringCaseInsensitive(StringRef s);

    /// Munch the rest of a tag after the tag name.
    /// attrs: storage for parsed attributes.
    /// selfEnding: if we want a self-ending tag.
    bool munchTagAfterTagName(std::vector<Attribute> &attrs, bool selfEnding = false);

    /// Munch a given char.
    bool munchChar(char c);

    /// Munch one arbitrary char.
    bool munchAny();

    /// Munch a space char.
    bool munchSpace();

    /// Munch a tag name in a tag.
    /// tagNameSink: storage for munched tag name.
    bool munchTagName(StringRef &tagNameSink);

    /// Munch an element attribute.
    bool munchAttribute(Attribute &attr);

    /// Munch a quoted string with the given quotation mark.
    bool munchQuotedString(char mark);

    /// If eof.
    [[nodiscard]] bool eof() const;

    /// Save the current lexer state.
    Savepoint checkpoint();
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

    std::vector<scb::String> words, titleWords;
    std::vector<Link> links;
    scb::String base;

    // The constructor is given a buffer and length containing
    // presumed HTML.  It will parse the buffer, stripping out
    // all the HTML tags and producing the list of words in body,
    // words in title, and links found on the page.
    HtmlParser(const scb::String &buffer);
protected:

    /// *** Parsing ***
    void parse();

    /// Split text by space and store the words into sink.
    static void splitText(StringRef s, std::vector<scb::String> &sink);

    /// *** Lexing ***
};
