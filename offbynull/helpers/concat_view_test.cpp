#include <ranges>
#include "offbynull/helpers/concat_view.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::concat_view::concat_view;

    TEST(ConcatViewTest, ForwardTest) {
        std::vector<int> vec1 {1, 2};
        std::vector<int> vec2 {3, 4};
        concat_view r{vec1, vec2};
        static_assert(std::ranges::forward_range<decltype(r)>);
        for (int n : r) {
            std::cout << n << ' ';
        }
        for (int n : r) {
            std::cout << n << ' ';
        }
    }

    TEST(ConcatViewTest, ReverseTest) {
        std::vector<int> vec1 {1, 2};
        std::vector<int> vec2 {3, 4};
        concat_view r{vec1, vec2};
        static_assert(std::ranges::forward_range<decltype(r)>);
        for (int n : r | std::views::reverse) {
            std::cout << n << ' ';
        }
        for (int n : r | std::views::reverse) {
            std::cout << n << ' ';
        }
    }

    TEST(ConcatViewTest, ChainTest) {
        std::vector<int> vec1 {1, 2};
        std::vector<int> vec2 {3, 4};
        std::vector<int> vec3 {5, 6};
        concat_view r{
            vec1,
            concat_view {vec2, vec3}
        };
        static_assert(std::ranges::forward_range<decltype(r)>);
        for (int n : r) {
            std::cout << n << ' ';
        }
        for (int n : r) {
            std::cout << n << ' ';
        }
    }

    TEST(ForwardRangeJoinViewTest, PipeTest) {
        // TODO: FIX CLASS TO SUPPORT PIPE OPERATOR
        // std::vector<std::vector<int>> vec {{1, 2}, {3, 4}, {5, 6}};
        // auto join_range {
        //     forward_range_join_view(
        //         std::move(vec)
        //     )
        //     | std::views::transform([](const auto&v) {return v*2;})
        // };
        // static_assert(std::ranges::forward_range<decltype(join_range)>);
        // for (int n : join_range) {
        //     std::cout << n << ' ';
        // }
        // for (int n : join_range) {
        //     std::cout << n << ' ';
        // }
    }
}