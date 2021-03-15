//
// Created by youh on 3/15/21.
//

#include "PorterStemming.h"


namespace porter {
    String label(const String &word) {
        // type of letters: consonant or vowel
        String labels(word.length(), 'c');
        for (int i = 0; i < word.length(); i++)
            switch (word[i]) {
                case 'a':
                case 'e':
                case 'i':
                case 'o':
                case 'u':
                    labels[i] = 'v';
                    break;
                case 'y':
                    if (i != 0 && word[i - 1] == 'c')
                        labels[i] = 'v';
                    break;
            }
        return labels;
    }

    int measure(const String &word) {
        String labels = label(word);

        // calculate measure m
        int m = 0;

        // skip Cs in the front
        int i = 0;
        for (; i < word.length() && labels[i] == 'c'; i++);

        while (i < word.length()) {
            // consume Vs
            for (; i < word.length() && labels[i] == 'v'; i++);
            if (i < word.length()) {
                m++;
                // consume Cs
                for (; i < word.length() && labels[i] == 'c'; i++);
            }
        }

        return m;
    }

    bool replaceSuffixIfMeasure(String &word, const String &suffix, const String &rpl, int m) {
        if (word.hasSuffix(suffix)) {
            String stem = word.substr(0, word.length() - suffix.length());
            if (measure(stem) > m) {
                for (char c: suffix)
                    word.popBack();
                word += rpl;
            }
            // see Porter himself's clarification at
            // https://tartarus.org/martin/PorterStemmer/def.txt
            return true;
        }
        return false;
    }

    void step1a(String &word) {
        if (word.hasSuffix("sses")) {
            word.popBack();
            word.popBack();
            return;
        }
        if (word.hasSuffix("ies")) {
            word.popBack();
            word.popBack();
            return;
        }
        if (word.hasSuffix("ss"))
            return;
        if (word.hasSuffix("s")) {
            word.popBack();
            return;
        }
    }

    bool step1b(String &word) {
        if (replaceSuffixIfMeasure(word, "eed", "ee"))
            return false;
        if (word.hasSuffix("ed")) {
            String stem = word.substr(0, word.length() - 2);
            if (label(stem).find("v") != String::npos) {
                word.popBack();
                word.popBack();
                return true;
            }
        }
        if (word.hasSuffix("ing")) {
            String stem = word.substr(0, word.length() - 3);
            if (label(stem).find("v") != String::npos) {
                word.popBack();
                word.popBack();
                word.popBack();
                return true;
            }
        }
        return false;
    }

    void step1bcont(String &word) {
        if (word.hasSuffix("at")) {
            word.pushBack('e');
            return;
        }
        if (word.hasSuffix("bl")) {
            word.pushBack('e');
            return;
        }
        if (word.hasSuffix("iz")) {
            word.pushBack('e');
            return;
        }

        if (word.length() >= 2) {
            String labels = label(word);
            int i2 = labels.size() - 1;
            int i1 = i2 - 1;
            if (labels[i1] == 'c' && labels[i2] == 'c' && word[i1] == word[i2] &&
                !(word[i1] == 'l' || word[i1] == 's' || word[i1] == 'z')) {
                word.popBack();
                return;
            }
        }

        if (measure(word) == 1) {
            String labels = label(word);
            if (labels.length() >= 3) {
                int i = labels.length() - 3;
                if (labels[i] == 'c' && labels[i + 1] == 'v' && labels[i + 2] == 'c')
                    switch (word[i + 2]) {
                        case 'w':
                        case 'x':
                        case 'y':
                            break;
                        default:
                            word.pushBack('e');
                            return;
                    }
            }
        }
    }

    void step1c(String &word) {
        if (word.hasSuffix("y")) {
            String stem = word.substr(0, word.length() - 1);
            if (label(stem).find("v") != String::npos)
                word.back() = 'i';
        }
    }

    void step2(String &word) {
        if (replaceSuffixIfMeasure(word, "ational", "ate"))
            return;
        if (replaceSuffixIfMeasure(word, "tional", "tion"))
            return;
        if (replaceSuffixIfMeasure(word, "enci", "ence"))
            return;
        if (replaceSuffixIfMeasure(word, "anci", "ance"))
            return;
        if (replaceSuffixIfMeasure(word, "izer", "ize"))
            return;
        if (replaceSuffixIfMeasure(word, "logi", "log"))
            return;
        if (replaceSuffixIfMeasure(word, "bli", "ble"))
            return;
        if (replaceSuffixIfMeasure(word, "alli", "al"))
            return;
        if (replaceSuffixIfMeasure(word, "entli", "ent"))
            return;
        if (replaceSuffixIfMeasure(word, "eli", "e"))
            return;
        if (replaceSuffixIfMeasure(word, "ousli", "ous"))
            return;
        if (replaceSuffixIfMeasure(word, "ization", "ize"))
            return;
        if (replaceSuffixIfMeasure(word, "ation", "ate"))
            return;
        if (replaceSuffixIfMeasure(word, "ator", "ate"))
            return;
        if (replaceSuffixIfMeasure(word, "alism", "al"))
            return;
        if (replaceSuffixIfMeasure(word, "iveness", "ive"))
            return;
        if (replaceSuffixIfMeasure(word, "fulness", "ful"))
            return;
        if (replaceSuffixIfMeasure(word, "ousness", "ous"))
            return;
        if (replaceSuffixIfMeasure(word, "aliti", "al"))
            return;
        if (replaceSuffixIfMeasure(word, "iviti", "ive"))
            return;
        if (replaceSuffixIfMeasure(word, "biliti", "ble"))
            return;
    }

    void step3(String &word) {
        if (replaceSuffixIfMeasure(word, "icate", "ic"))
            return;
        if (replaceSuffixIfMeasure(word, "ative", ""))
            return;
        if (replaceSuffixIfMeasure(word, "alize", "al"))
            return;
        if (replaceSuffixIfMeasure(word, "iciti", "ic"))
            return;
        if (replaceSuffixIfMeasure(word, "ical", "ic"))
            return;
        if (replaceSuffixIfMeasure(word, "ful", ""))
            return;
        if (replaceSuffixIfMeasure(word, "ness", ""))
            return;
    }

    void step4(String &word) {
        if (replaceSuffixIfMeasure(word, "al", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ance", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ence", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "er", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ic", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "able", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ible", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ant", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ement", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ment", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ent", "", 1))
            return;
        if (word.hasSuffix("ion")) {
            String stem = word.substr(0, word.length() - 3);
            if (stem.back() == 's' || stem.back() == 't') {
                if (measure(stem) > 1) {
                    word = stem;
                    return;
                }
            }
        }
        if (replaceSuffixIfMeasure(word, "ou", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ism", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ate", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "iti", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ous", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ive", "", 1))
            return;
        if (replaceSuffixIfMeasure(word, "ize", "", 1))
            return;
    }

    void step5a(String &word) {
        if (word.empty() || word.back() != 'e')
            return;

        String stem = word.substr(0, word.length() - 1);
        int m = measure(stem);

        if (m > 1) {
            word.popBack();
            return;
        }

        if (m == 0)
            return;

        // m == 1
        auto labels = label(stem);
        if (labels.length() >= 3) {
            int i = labels.length() - 3;
            if (labels[i] == 'c' && labels[i + 1] == 'v' && labels[i + 2] == 'c')
                return;
        }

        word.popBack();
    }

    void step5b(String &word) {
        int m = measure(word);
        if (m <= 1)
            return;

        if (word.length() < 2)
            return;

        String tail = word.substr(word.length() - 2);

        if (tail == "ll") {
            word.popBack();
            return;
        }
    }
}

String PorterStem(const String &input) {
    String word;

    // convert to lower case and remove non-alphanum characters
    for (char &c: input)
        if (isalnum(c))
            word.pushBack(tolower(c));

    if (word.length() <= 2)
        return word;

    // step 1
    porter::step1a(word);
    if (porter::step1b(word))
        porter::step1bcont(word);
    porter::step1c(word);

    porter::step2(word);

    porter::step3(word);

    porter::step4(word);

    porter::step5a(word);
    porter::step5b(word);

    return word;
}
