#include "offbynull/aligner/scorers/pam250_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>

namespace {
    using offbynull::aligner::scorers::pam250_scorer::pam250_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASPam250ScorerTest, SanityTest) {
        pam250_scorer<is_debug_mode(), int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(2, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-2, (scorer(std::tuple<> {}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(12, (scorer(std::tuple<> {}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(-2, (scorer(std::tuple<> {}, { { c_ } }, { { a_ } })));
    }
}