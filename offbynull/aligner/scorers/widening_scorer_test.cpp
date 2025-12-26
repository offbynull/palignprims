#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/aligner/scorers/widening_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace {
    using offbynull::aligner::scorers::widening_scorer::widening_scorer;
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASWideningScorerTest, SanityTest) {
        constant_scorer<
            is_debug_mode(),
            std::size_t,
            char,
            char,
            float
        > backing_scorer { 5.0f };
        widening_scorer<
            is_debug_mode(),
            std::uint8_t,
            char,
            char,
            double,
            decltype(backing_scorer)
        > scorer { backing_scorer };
        char a_ { 'a' };
        char b_ { 'b' };
        char z_ { 'z' };
        char c_ { 'c' };
        std::cout << "test test" << scorer({ { 0zu, { a_ } } }, { { 0zu, { b_ } } });
        EXPECT_EQ(5.0, (scorer({ { 0zu, { a_ } } }, { { 0zu, { b_ } } })));
        EXPECT_EQ(5.0, (scorer({ { 0zu, { z_ } } }, { { 0zu, { c_ } } })));
    }
}