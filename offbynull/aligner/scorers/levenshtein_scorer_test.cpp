#include "offbynull/aligner/scorers/levenshtein_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>
#include <optional>

namespace {
    using offbynull::aligner::scorers::levenshtein_scorer::levenshtein_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASLevenshteinScorerTest, SanityTest) {
        levenshtein_scorer<is_debug_mode(), char, char, int> scorer {};
        char a_ { 'a' };
        char b_ { 'b' };
        EXPECT_EQ(-1, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { a_ } }, { { b_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { a_ } }, { std::nullopt })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { std::nullopt }, { { b_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { std::nullopt }, { std::nullopt })));
    }
}