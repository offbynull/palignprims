#include <ranges>
#include <iostream>
#include "offbynull/helpers/blankable_bidirectional_view.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view;
    using offbynull::utils::is_debug_mode;

    TEST(OHBlankableBidirectionalViewTest, PassthroughTest) {
        blankable_bidirectional_view r_ { true, std::views::iota(1, 10) };
        EXPECT_NE(r_.begin(), r_.end());
        for (int n : r_) {
            std::cout << n << ' ';
        }
    }

    TEST(OHBlankableBidirectionalViewTest, BlankTest) {
        blankable_bidirectional_view r_ { false, std::views::iota(1, 10) };
        EXPECT_EQ(r_.begin(), r_.end());
        for (int n : r_) {
            std::cout << n << ' ';
        }
    }
}