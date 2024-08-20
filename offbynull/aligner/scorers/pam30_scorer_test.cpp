#include "offbynull/aligner/scorers/pam30_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::pam30_scorer::pam30_scorer;

    TEST(OASPam30ScorerTest, SanityTest) {
        pam30_scorer<true, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(6, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-6, (scorer(std::tuple<> {}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(10, (scorer(std::tuple<> {}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(-6, (scorer(std::tuple<> {}, { { c_ } }, { { a_ } })));
    }
}