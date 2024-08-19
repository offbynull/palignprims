#include "offbynull/aligner/scorers/constant_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;

    TEST(ConstantScorerTest, SanityTest) {
        constant_scorer<true, int> scorer { 5 };
        EXPECT_EQ(5, (scorer(std::tuple<> {}, 'a', 'b')));
        EXPECT_EQ(5, (scorer(std::tuple<> {}, 'z', 'c')));
    }
}