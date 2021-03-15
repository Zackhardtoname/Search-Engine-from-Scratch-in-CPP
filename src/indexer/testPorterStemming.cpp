//
// Created by youh on 3/15/21.
//

#include <fstream>
#include <string>

#include "PorterStemming.h"
#include "../stl/include/String.h"
#include "gtest/gtest.h"

TEST(PorterStemmingTest, CheckNLTKTestCase) {
    std::fstream vocabText("testPorterVoc.txt"), refText("testPorterRef.txt");
    std::string original, ref;
    while ((vocabText >> original) && (refText >> ref)) {
        ASSERT_STREQ(PorterStem(original.c_str()).cstr(), ref.c_str());
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
