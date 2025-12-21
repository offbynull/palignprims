#include "offbynull/aligner/scorers/levenshtein_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>
#include <optional>
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::levenshtein_scorer::levenshtein_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASLevenshteinScorerTest, SanityTest) {
        levenshtein_scorer<is_debug_mode(), std::size_t, char, char, int> scorer {};
        char a_ { 'a' };
        char b_ { 'b' };
        EXPECT_EQ(-1, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { b_ } } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { std::nullopt })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { std::nullopt }, { { 0zu, { b_ } } })));
        EXPECT_EQ(0, (scorer(std::tuple<> {}, { std::nullopt }, { std::nullopt })));
    }
}