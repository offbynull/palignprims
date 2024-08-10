#include "offbynull/aligner/scorers/pam250_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::pam250_scorer::pam250_scorer;

    TEST(Pam250ScorerTest, SanityTest) {
        pam250_scorer<true, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(2, (scorer(std::tuple<>{}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-2, (scorer(std::tuple<>{}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(12, (scorer(std::tuple<>{}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(-2, (scorer(std::tuple<>{}, { { c_ } }, { { a_ } })));
    }
}