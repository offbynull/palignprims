#include "offbynull/aligner/scorers/blosum_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::blosum_scorer::blosum_scorer;
    using offbynull::aligner::scorers::blosum_scorer::threshold;
    using offbynull::utils::is_debug_mode;

    TEST(OASBlosumScorerTest, SanityTest45) {
        blosum_scorer<is_debug_mode(), threshold::_45, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(12, (scorer({ { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }

    TEST(OASBlosumScorerTest, SanityTest50) {
        blosum_scorer<is_debug_mode(), threshold::_50, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(13, (scorer({ { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }

    TEST(OASBlosumScorerTest, SanityTest62) {
        blosum_scorer<is_debug_mode(), threshold::_62, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(4, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(0, (scorer({ { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(9, (scorer({ { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(0, (scorer({ { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }

    TEST(OASBlosumScorerTest, SanityTest80) {
        blosum_scorer<is_debug_mode(), threshold::_80, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(9, (scorer({ { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }

    TEST(OASBlosumScorerTest, SanityTest90) {
        blosum_scorer<is_debug_mode(), threshold::_90, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(9, (scorer({ { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }
}