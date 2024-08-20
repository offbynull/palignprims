#include <ranges>
#include "offbynull/helpers/simple_value_bidirectional_view.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::simple_value_bidirectional_view::simple_value_bidirectional_view;
    using offbynull::helpers::simple_value_bidirectional_view::iterator;

    TEST(OHSimpleValueBidirectionalViewTest, SanityTest) {
        struct state {
            int value_;

            void to_prev() {
                value_ -= 1;
            }

            void to_next() {
                value_ += 1;
            }

            int value() const {
                return value_;
            }

            bool operator==(const state& other) const = default;
        };

        simple_value_bidirectional_view<state> r {
            state { 5 },
            state { 10 }
        };
        for (const auto& x : r) {
            std::cout << x << ' ';
        }
        std::cout << std::endl;
        for (const auto& x : r | std::views::reverse) {
            std::cout << x << ' ';
        }
        std::cout << std::endl;
    }
}