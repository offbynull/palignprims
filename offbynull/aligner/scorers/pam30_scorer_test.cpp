#include "offbynull/aligner/scorers/pam30_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>

namespace {
    using offbynull::aligner::scorers::pam30_scorer::pam30_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASPam30ScorerTest, SanityTest) {
        pam30_scorer<is_debug_mode(), int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(6, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-6, (scorer(std::tuple<> {}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(10, (scorer(std::tuple<> {}, { { c_ } }, { { c_ } })));
        EXPECT_EQ(-6, (scorer(std::tuple<> {}, { { c_ } }, { { a_ } })));
    }
}