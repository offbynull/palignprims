#include <ranges>
#include <utility>
#include <vector>
#include "offbynull/helpers/join_bidirectional_view.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::join_bidirectional_view::join_bidirectional_view;
    using offbynull::helpers::join_bidirectional_view::join_bidirectional;
    using offbynull::utils::copy_to_vector;

    TEST(OHJoinBidirectionalViewTest, WithVectorTest) {
        std::vector<std::vector<int>> vec { {1, 2}, {3, 4}, {5, 6} };
        join_bidirectional_view<decltype(vec)> join_range(std::move(vec));
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> {1, 2, 3, 4, 5, 6})
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> {6, 5, 4, 3, 2, 1})
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithStartingEmptiesTest) {
        std::vector<std::vector<int>> vec { {}, {}, {1, 2}, {3, 4}, {5, 6}};
        join_bidirectional_view<decltype(vec)> join_range(std::move(vec));
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> { 1, 2, 3, 4, 5, 6 })
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> { 6, 5, 4, 3, 2, 1 })
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithEndingEmptiesTest) {
        std::vector<std::vector<int>> vec { {}, {}, {1, 2}, {3, 4}, {5, 6}};
        join_bidirectional_view<decltype(vec)> join_range(std::move(vec));
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> { 1, 2, 3, 4, 5, 6 })
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> { 6, 5, 4, 3, 2, 1 })
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithSplicedEmptiesTest) {
        std::vector<std::vector<int>> vec { {}, {1, 2}, {}, {3, 4}, {}, {5, 6}, {} };
        join_bidirectional_view<decltype(vec)> join_range(std::move(vec));
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> { 1, 2, 3, 4, 5, 6 })
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> { 6, 5, 4, 3, 2, 1 })
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithInnerBeingSingleEmptyTest) {
        std::vector<std::vector<int>> vec { {} };
        join_bidirectional_view<decltype(vec)> join_range(std::move(vec));
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> {})
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> {})
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithTotallyEmptyTest) {
        std::vector<std::vector<int>> vec {};
        join_bidirectional_view<decltype(vec)> join_range(std::move(vec));
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> {})
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> {})
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithVectorPipeTest) {
        auto join_range {
            std::views::iota(0, 3)
            | std::views::transform([](const int i) {
                return std::views::iota(i*2+1, i*2+3);
            })
            | join_bidirectional()
        };
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> { 1, 2, 3, 4, 5, 6 })
        );
        EXPECT_EQ(
            copy_to_vector(join_range | std::views::reverse),
            (std::vector<int> { 6, 5, 4, 3, 2, 1 })
        );
    }

    TEST(OHJoinBidirectionalViewTest, WithIotaViewPipeTest) {
        auto join_range {
            std::vector<std::vector<int>> { {1, 2}, {3, 4}, {5, 6} }
            | join_bidirectional()
        };
        EXPECT_EQ(
            copy_to_vector(join_range),
            (std::vector<int> { 1, 2, 3, 4, 5, 6 })
        );
        EXPECT_EQ(
            copy_to_vector(std::move(join_range) | std::views::reverse),
            (std::vector<int> { 6, 5, 4, 3, 2, 1 })
        );
    }
}