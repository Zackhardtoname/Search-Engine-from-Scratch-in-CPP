#include <ctype.h>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <string>
#include "HtmlTags.h"

// name points to beginning of the possible HTML tag name.
// nameEnd points to one past last character.
// Comparison is case-insensitive.
// Use a binary search.
// If the name is found in the TagsRecognized table, return
// the corresponding action.
// If the name is not found, return OrdinaryText.

int tagCmp(const void *a, const void *b) {
    return strcmp(static_cast<const HtmlTag *>(a)->Tag, static_cast<const HtmlTag *>(b)->Tag);
}

DesiredAction LookupPossibleTag(const char *name, const char *nameEnd) {
    // TODO: case insensitive
    std::string wrappedNameString(name, nameEnd);
    HtmlTag targetTag = {
            wrappedNameString.c_str(),
            DesiredAction::OrdinaryText
    };
    if (void *foundTag = bsearch(&targetTag, TagsRecognized, NumberOfTags, sizeof(HtmlTag), tagCmp))
        return static_cast<HtmlTag *>(foundTag)->Action;
    return DesiredAction::OrdinaryText;
}
