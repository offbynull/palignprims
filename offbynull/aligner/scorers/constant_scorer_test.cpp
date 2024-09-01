#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>

namespace {
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASConstantScorerTest, SanityTest) {
        constant_scorer<is_debug_mode(), int> scorer { 5 };
        EXPECT_EQ(5, (scorer(std::tuple<> {}, 'a', 'b')));
        EXPECT_EQ(5, (scorer(std::tuple<> {}, 'z', 'c')));
    }
}