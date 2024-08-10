#include "offbynull/aligner/scorers/blosum80_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::blosum80_scorer::blosum80_scorer;

    TEST(Blosum80ScorerTest, SanityTest) {
        blosum80_scorer<true, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer(std::tuple<>{}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-1, (scorer(std::tuple<>{}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(9, (scorer(std::tuple<>{}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(-1, (scorer(std::tuple<>{}, { { c_ } }, { { a_ } })));
    }
}