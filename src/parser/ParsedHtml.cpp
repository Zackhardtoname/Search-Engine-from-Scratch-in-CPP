// HtmlParser.cpp
// Nicole Hamlton, nham@umich.edu

#include <cassert>
#include <unordered_map>
#include <iostream>
#include "../stl/include/io/utils.h"
#include "../stl/include/String.h"

#include "ParsedHtml.h"
#include "HtmlLexer.h"

ParsedHtml::ParsedHtml(const String &buffer) {
    HtmlLexer lexer(buffer);

    // pendingLinks: Index to Link objects that have not been finalized.
    // This is a stack because even though <a> are not supposed to nest,
    // inexperienced developers may still do that and we want to tolerate that.
    // Since this refers to things in a vector, we have to use indices.
    Vector<unsigned> pendingLinks;

    while (lexer.HasNextEntity()) {
        HtmlEntity &entity = lexer.NextEntity();

        // Handle <base>
        // I've seen people write base as both <base> and <base/>
        // So handle both here.
        if (entity.TagName.EqualsCaseInsensitive("base") && Base.empty()) {
            if (auto *attr = entity.FindAttr("href"))
                Base = attr->second.Str();
            continue;
        }

        // Handle <embed>
        // I imagine people do the same for <embed>
        if (entity.TagName.EqualsCaseInsensitive("embed")) {
            if (auto *attr = entity.FindAttr("src"))
                Links.emplaceBack(attr->second.Str());
            continue;
        }

        // Handle <html>
        if (entity.TagName.EqualsCaseInsensitive("html")) {
            if (auto *attr = entity.FindAttr("lang"))
                Language = attr->second.Str();
            continue;
        }

        // Handle other tags.
        switch (entity.Type) {
            case HtmlEntity::StartTag:
                if (entity.TagName.EqualsCaseInsensitive("a")) {
                    String href;
                    if (auto *attr = entity.FindAttr("href"))
                        href = attr->second.Str();
                    pendingLinks.pushBack((unsigned) Links.size());
                    Links.emplaceBack(href);
                }
                break;
            case HtmlEntity::RawTextElement:
                // only care about textarea and title.
                // this may be inside an <a> but that's rare, don't bother.
                if (entity.TagName.EqualsCaseInsensitive("title"))
                    splitText(entity.InnerText, TitleWords);
                if (entity.TagName.EqualsCaseInsensitive("textarea"))
                    splitText(entity.InnerText, Words);
                break;
            case HtmlEntity::EndTag:
                if (entity.TagName.EqualsCaseInsensitive("a") && !pendingLinks.empty())
                    pendingLinks.popBack();
                break;
            case HtmlEntity::SelfEndingTag:
                break;
            case HtmlEntity::Text:
                splitText(entity.Content, Words);
                if (!pendingLinks.empty())
                    splitText(entity.Content, Links[pendingLinks.back()].AnchorText);
                break;
            case HtmlEntity::Comment:
                break;
        }
    }
}


void ParsedHtml::splitText(StringRef s, Vector<String> &sink) {
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
