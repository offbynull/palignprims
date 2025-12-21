#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASConstantScorerTest, SanityTest) {
        constant_scorer<is_debug_mode(), std::size_t, char, char, int> scorer { 5 };
        char a_ { 'a' };
        char b_ { 'b' };
        char z_ { 'z' };
        char c_ { 'c' };
        EXPECT_EQ(5, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { b_ } } })));
        EXPECT_EQ(5, (scorer(std::tuple<> {}, { { 0zu, { z_ } } }, { { 0zu, { c_ } } })));
    }
}