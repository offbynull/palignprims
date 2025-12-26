#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <optional>
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASSimpleScorerTest, SanityTest) {
        simple_scorer<is_debug_mode(), std::size_t, char, char, int> scorer { 1, 0, -1, -1, 0 };
        char a_ { 'a' };
        char b_ { 'b' };
        EXPECT_EQ(1, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(0, (scorer({ { 0zu, { a_ } } }, { { 0zu, { b_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { a_ } } }, { std::nullopt })));
        EXPECT_EQ(-1, (scorer({ std::nullopt }, { { 0zu, { b_ } } })));
        EXPECT_EQ(0, (scorer({ std::nullopt }, { std::nullopt })));
    }
}