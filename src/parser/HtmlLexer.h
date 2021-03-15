//
// Created by youh on 3/14/21.
//

#ifndef SCOOBLE_SEARCH_HTMLLEXER_H
#define SCOOBLE_SEARCH_HTMLLEXER_H

#include "../stl/include/String.h"

using scb::String;
using scb::StringRef;
using scb::Vector;

// TODO: make and use our own pair
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
    Vector<Attribute> Attributes;
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
        String::Iterator save;
    public:

        /// Discard the saved state.
        void Commit();

        Savepoint(HtmlLexer &lexer, String::Iterator save);

        ~Savepoint();
    };

    String::Iterator cursor, bufEnd;

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
    bool munchTagAfterTagName(Vector<Attribute> &attrs, bool selfEnding = false);

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
#endif //SCOOBLE_SEARCH_HTMLLEXER_H
