#include "offbynull/aligner/scorers/qwerty_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>
#include <iostream>
#include <ostream>
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::qwerty_scorer::qwerty_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASQwertyScorerTest, SanityTest) {
        qwerty_scorer<is_debug_mode(), std::size_t, float> scorer {};
        {
            char ch1_ { 'A' };
            char ch2_ { 'A' };
            EXPECT_NEAR(1.9f, (scorer(std::tuple<> {}, { { 0zu, { ch1_ } } }, { { 0zu, { ch2_ } } })), 0.0001f);
        }
        {
            char ch1_ { 'A' };
            char ch2_ { 'W' };
            EXPECT_NEAR(0.69796f, (scorer(std::tuple<> {}, { { 0zu, { ch1_ } } }, { { 0zu, { ch2_ } } })), 0.0001f);
        }
        {
            char ch1_ { 'A' };
            char ch2_ { 'S' };
            EXPECT_NEAR(0.9f, (scorer(std::tuple<> {}, { { 0zu, { ch1_ } } }, { { 0zu, { ch2_ } } })), 0.0001f);
        }
        {
            char ch1_ { 'A' };
            char ch2_ { 'X' };
            EXPECT_NEAR(0.2327f, (scorer(std::tuple<> {}, { { 0zu, { ch1_ } } }, { { 0zu, { ch2_ } } })), 0.0001f);
        }
        {
            char ch1_ { 'A' };
            char ch2_ { ']' };
            EXPECT_NEAR(-8.81377f, (scorer(std::tuple<> {}, { { 0zu, { ch1_ } } }, { { 0zu, { ch2_ } } })), 0.0001f);
        }

        for (char ch1_ : { 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/' }) {
            char ch2_ { ' ' };
            std::cout << scorer(std::tuple<> {}, { { 0zu, { ch1_ } } }, { { 0zu, { ch2_ } } }) << std::endl;
        }
    }
}