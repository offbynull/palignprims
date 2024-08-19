#include "offbynull/aligner/scorers/levenshtein_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::levenshtein_scorer::levenshtein_scorer;

    TEST(LevenshteinScorerTest, SanityTest) {
        levenshtein_scorer<true, char, char, int> scorer {};
        char a_ { 'a' };
        char b_ { 'b' };
        EXPECT_EQ(-1, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { a_ } }, { { b_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { a_ } }, { std::nullopt })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { std::nullopt }, { { b_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { std::nullopt }, { std::nullopt })));
    }
}