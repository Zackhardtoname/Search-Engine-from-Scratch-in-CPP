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

using scb::String;

class EnglishDictChecker {
    constexpr static float ENGLISH_THRESHOLD = 0.6;

    std::unordered_set<String, StringHash> vocab;
public:
    bool Check(const ParsedHtml &parsedHtml) {
        int hit = 0;
        for (String &word: parsedHtml.Words)
            if (vocab.contains(word))
                hit++;
        return hit > parsedHtml.Words.size() * ENGLISH_THRESHOLD;
    }

    explicit EnglishDictChecker(const char *refFile) {
        std::fstream f(refFile);
        std::string word;
        while (f >> word)
            vocab.insert(String(word.c_str()));
    }
};


#endif //SCOOBLE_SEARCH_ENGLISHDICTCHECKER_H
