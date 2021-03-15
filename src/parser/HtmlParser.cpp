// HtmlParser.cpp
// Nicole Hamlton, nham@umich.edu

#include <cassert>
#include <unordered_map>
#include <iostream>
#include <stack>
#include "../stl/include/io/utils.h"
#include "../stl/include/String.h"

#include "HtmlParser.h"
#include "HtmlLexer.h"

HtmlParser::HtmlParser(const String &buffer) {
    HtmlLexer lexer(buffer);

    // pendingLinks: Index to Link objects that have not been finalized.
    // This is a stack because even though <a> are not supposed to nest,
    // inexperienced developers may still do that and we want to tolerate that.
    // Since this refers to things in a vector, we have to use indices.
    std::stack<unsigned> pendingLinks;

    while (lexer.HasNextEntity()) {
        HtmlEntity &entity = lexer.NextEntity();

        // I've seen people write base as both <base> and <base/>
        // So handle both here.
        if (entity.TagName.EqualsCaseInsensitive("base") && base.empty()) {
            for (const auto &attr: entity.Attributes)
                if (attr.first.EqualsCaseInsensitive("href")) {
                    base = attr.second.Str();
                    break;
                }
            continue;
        }
        // I imagine people do the same for <embed>
        if (entity.TagName.EqualsCaseInsensitive("embed")) {
            for (const auto &attr: entity.Attributes)
                if (attr.first.EqualsCaseInsensitive("src")) {
                    links.emplaceBack(attr.second.Str());
                    break;
                }
            continue;
        }
        switch (entity.Type) {
            case HtmlEntity::StartTag:
                if (entity.TagName.EqualsCaseInsensitive("a")) {
                    String href;
                    for (const auto &attr: entity.Attributes)
                        if (attr.first.EqualsCaseInsensitive("href")) {
                            href = attr.second.Str();
                            break;
                        }
                    pendingLinks.push((unsigned) links.size());
                    links.emplaceBack(href);
                }
                break;
            case HtmlEntity::RawTextElement:
                // only care about textarea and title.
                // this may be inside an <a> but that's rare, don't bother.
                if (entity.TagName.EqualsCaseInsensitive("title"))
                    splitText(entity.InnerText, titleWords);
                if (entity.TagName.EqualsCaseInsensitive("textarea"))
                    splitText(entity.InnerText, words);
                break;
            case HtmlEntity::EndTag:
                if (entity.TagName.EqualsCaseInsensitive("a") && !pendingLinks.empty())
                    pendingLinks.pop();
                break;
            case HtmlEntity::SelfEndingTag:
                break;
            case HtmlEntity::Text:
                splitText(entity.Content, words);
                if (!pendingLinks.empty())
                    splitText(entity.Content, links[pendingLinks.top()].anchorText);
                break;
            case HtmlEntity::Comment:
                break;
        }
    }
}


void HtmlParser::splitText(StringRef s, Vector<String> &sink) {
    String buf;

    for (String::Iterator i = s.Begin; i != s.End; i++) {
        char c = *i;
        if (isspace(c)) {
            if (!buf.empty()) {
                sink.pushBack(std::move(buf));
                buf.clear();
            }
        } else
            buf.pushBack(c);
    }

    if (!buf.empty())
        sink.pushBack(std::move(buf));
}
