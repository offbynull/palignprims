#include <ranges>
#include "offbynull/helpers/blankable_bidirectional_view.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view;

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