#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <stdfloat>
#include <cstdint>
#include "offbynull/aligner/aligners/rotational_dynamic_programming_stack_aligner.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/aligner/aligners/utils.h"
#include "offbynull/aligner/aligners/concepts.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::aligners::rotational_dynamic_programming_stack_aligner::rotational_dynamic_programming_stack_aligner;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::aligner::aligners::utils::alignment_to_stacked_string;
    using offbynull::utils::is_debug_mode;

    TEST(OAARotationalDynamicProgrammingStackAlignerTest, SanityTest) {
        rotational_dynamic_programming_stack_aligner<is_debug_mode()> aligner {};
        auto substitution_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, int>::create_substitution(1, -1) };
        auto gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, int>::create_gap(-1) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, int>::create_freeride() };
        std::string down { "lo world!hel" };
        std::string right { "mellow" };
        const auto& [alignment, score] {
            aligner.align<20zu, 10zu>(down, right, substitution_scorer, gap_scorer, freeride_scorer)
        };
        EXPECT_EQ(3, score);
        std::cout << score << std::endl;
        std::cout << alignment_to_stacked_string<is_debug_mode()>(down, right, alignment) << std::endl;
    }

    TEST(OAARotationalDynamicProgrammingStackAlignerTest, ParameterizationTest) {
        rotational_dynamic_programming_stack_aligner<is_debug_mode()> aligner {};

        std::string down { "lo world!hel" };
        std::string right { "mellow" };

        auto align_ {
            [&]<typename N_INDEX, typename WEIGHT>() {
                auto substitution_scorer {
                    simple_scorer<is_debug_mode(), N_INDEX, char, char, WEIGHT>::create_substitution(
                        static_cast<WEIGHT>(1),
                        static_cast<WEIGHT>(-1)
                    )
                };
                auto gap_scorer {
                    simple_scorer<is_debug_mode(), N_INDEX, char, char, WEIGHT>::create_gap(
                        static_cast<WEIGHT>(-1)
                    )
                };
                auto freeride_scorer {
                    simple_scorer<is_debug_mode(), N_INDEX, char, char, WEIGHT>::create_freeride()
                };
                const auto& [alignment, score] {
                    aligner.align_strict<N_INDEX, WEIGHT, 20zu, 10zu>(down, right, substitution_scorer, gap_scorer, freeride_scorer)
                };
                return score;
            }
        };

        // small index type
        EXPECT_EQ(3, (align_.operator()<std::uint8_t, std::int8_t>()));
        EXPECT_EQ(3, (align_.operator()<std::uint8_t, std::int16_t>()));
        EXPECT_EQ(3, (align_.operator()<std::uint8_t, std::int32_t>()));
        EXPECT_EQ(3, (align_.operator()<std::uint8_t, std::int64_t>()));
        EXPECT_EQ(3, (align_.operator()<std::uint8_t, std::float32_t>()));
        EXPECT_EQ(3, (align_.operator()<std::uint8_t, std::float64_t>()));
        // large index type
        EXPECT_EQ(3, (align_.operator()<std::size_t, std::int8_t>()));
        EXPECT_EQ(3, (align_.operator()<std::size_t, std::int16_t>()));
        EXPECT_EQ(3, (align_.operator()<std::size_t, std::int32_t>()));
        EXPECT_EQ(3, (align_.operator()<std::size_t, std::int64_t>()));
        EXPECT_EQ(3, (align_.operator()<std::size_t, std::float32_t>()));
        EXPECT_EQ(3, (align_.operator()<std::size_t, std::float64_t>()));
    }
}