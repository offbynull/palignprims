#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/aligner/scorers/wrap_callable_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <optional>
#include <utility>
#include <functional>
#include <type_traits>

namespace {
    using offbynull::aligner::scorers::wrap_callable_scorer::wrap_callable_scorer;
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASWrapCallableScorerTest, SanityTest) {
        auto callable {
            [](
                const std::optional<
                    std::pair<
                        std::uint16_t,
                        std::reference_wrapper<const char>
                    >
                > down_elem,
                const std::optional<
                    std::pair<
                        std::uint64_t,
                        std::reference_wrapper<const int>
                    >
                > right_elem
            ) {
                const auto& [d_idx, d_elem] { *down_elem };
                const auto& [r_idx, r_elem] { *right_elem };
                return static_cast<std::uint8_t>(d_elem - r_elem);
            }
        };
        wrap_callable_scorer<is_debug_mode(), std::remove_cvref_t<decltype(callable)>> scorer { callable };
        char a_ { 10 };
        int b_ { 5 };
        char z_ { 20 };
        int c_ { 10 };
        static_assert(std::is_same_v<decltype(scorer)::WEIGHT, std::uint8_t>);
        EXPECT_EQ(5, (scorer({ { static_cast<std::uint16_t>(0), { a_ } } }, { { static_cast<std::uint16_t>(0), { b_ } } })));
        EXPECT_EQ(10, (scorer({ { static_cast<std::uint16_t>(0), { z_ } } }, { { static_cast<std::uint16_t>(0), { c_ } } })));
    }
}