//
// Created by youh on 3/15/21.
//

#ifndef SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H
#define SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H

#include <fstream>
#include <string>
#include <unordered_set>

#include "../parser/ParsedHtml.h"
#include "../stl/include/String.h"
#include "../indexer/IndexChunk.h"
#include "../indexer/PorterStemming.h"

using scb::String;

/// Check if a given HTML is in English by comparing the words
/// with those in a given dictionary. The dictionary should not
/// be stemmed, but any HTML passed in is assumed to have been
/// stemmed.
class EnglishDictChecker {
    constexpr static float ENGLISH_THRESHOLD = 0.35;

    // TODO: change this to our own unordered_set once we have one
    std::unordered_set<String, StringHash> vocab;
public:
    // Check if the given HTML is in English. Assuming
    // the words in the document have been stemmed.
    bool Check(const ParsedHtml &parsedHtml) {
        int hit = 0;
        for (String &word: parsedHtml.Words)
            if (vocab.count(word))
                hit++;

        // it is in English if at least this proportion of words are English
        return hit > parsedHtml.Words.size() * ENGLISH_THRESHOLD;
    }

    explicit EnglishDictChecker(const char *vocabFile) {
        std::fstream f(vocabFile);
        // read in words from the dictionary and stem them
        std::string word;
        while (f >> word) {
            String stem = PorterStem(String(word.c_str()));
            vocab.insert(stem);
        }
        assert(!vocab.empty() && "empty vocabulary");
    }
};


#endif //SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H
