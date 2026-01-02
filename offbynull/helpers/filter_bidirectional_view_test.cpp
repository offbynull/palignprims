#include <ranges>
#include <iostream>
#include <ostream>
#include <vector>
#include <ranges>
#include "offbynull/helpers/filter_bidirectional_view.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::filter_bidirectional_view::filter_bidirectional_view;
    using offbynull::helpers::filter_bidirectional_view::filter_bidirectional;
    using offbynull::utils::is_debug_mode;

    TEST(OHFilterBidirectionalViewTest, PassthroughTest) {
        {
            filter_bidirectional_view r_ { std::views::iota(1, 10), [](auto x) { return x % 2 == 0; } };
            static_assert(std::ranges::bidirectional_range<decltype(r_)>);
            for (int n : r_) {
                std::cout << n << ' ';
            }
            std::cout << std::endl;
        }
        {
            for (int n : std::views::iota(1, 10)
                    | std::views::transform([](const auto x) { return x * 2; })
                    | filter_bidirectional([](auto x) { return x % 2 == 0; })
                    | std::views::reverse) {
                std::cout << n << ' ';
            }
            std::cout << std::endl;
        }
        {
            int y = 2;
            for (int n : std::views::iota(1, 10)
                    | std::views::transform([](const auto x) { return x * 2; })
                    | filter_bidirectional([y](auto x) { return x % y == 0; })
                    | std::views::reverse) {
                std::cout << n << ' ';
            }
            std::cout << std::endl;
        }
        {
            filter_bidirectional_view r_ { std::vector<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9 }, [](auto x) { return x % 2 == 0; } };
            static_assert(std::ranges::bidirectional_range<decltype(r_)>);
            for (int n : r_) {
                std::cout << n << ' ';
            }
            std::cout << std::endl;
        }
        {
            for (int n : std::vector<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9 }
                    | std::views::transform([](const auto x) { return x * 2; })
                    | filter_bidirectional([](auto x) { return x % 2 == 0; })
                    | std::views::reverse) {
                std::cout << n << ' ';
            }
            std::cout << std::endl;
        }
    }
}