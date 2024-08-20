#include "offbynull/aligner/scorers/blosum62_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::blosum62_scorer::blosum62_scorer;

    TEST(OASBlosum62ScorerTest, SanityTest) {
        blosum62_scorer<true, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(4, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(9, (scorer(std::tuple<> {}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { c_ } }, { { a_ } })));
    }
}