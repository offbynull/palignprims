#include "offbynull/aligner/scorers/blosum45_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>

namespace {
    using offbynull::aligner::scorers::blosum45_scorer::blosum45_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASBlosum45ScorerTest, SanityTest) {
        blosum45_scorer<is_debug_mode(), int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-1, (scorer(std::tuple<> {}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(12, (scorer(std::tuple<> {}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(-1, (scorer(std::tuple<> {}, { { c_ } }, { { a_ } })));
    }
}