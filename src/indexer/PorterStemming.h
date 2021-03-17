//
// Created by youh on 3/15/21.
//

#ifndef SCOOBLE_SEARCH_PORTERSTEMMING_H
#define SCOOBLE_SEARCH_PORTERSTEMMING_H

#include "../stl/String.h"
#include "../stl/Vector.h"

using scb::String;
using scb::Vector;

namespace porter {
    String label(const String &word);

    int measure(const String &word);

    // the *o construct
    bool staro(const String& stem);

    // the common construct of (m>x) S1 -> S2
    bool suffixMeasure(String &word, const String &suffix, const String &rpl, int m = 0);

    void step1a(String &word);

    // Returns true if the second/third step succeeded.
    bool step1b(String &word);

    void step1bcont(String &word);

    void step1c(String &word);

    void step2(String &word);

    void step3(String &word);

    void step4(String &word);

    void step5a(String &word);

    void step5b(String &word);
}

// https://tartarus.org/martin/PorterStemmer/def.txt
// and https://tartarus.org/martin/PorterStemmer/index.html
String PorterStem(const String &word);

#endif //SCOOBLE_SEARCH_PORTERSTEMMING_H
