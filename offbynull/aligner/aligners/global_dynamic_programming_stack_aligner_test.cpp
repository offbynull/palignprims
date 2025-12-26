#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <stdfloat>
#include <cstdint>
#include "offbynull/aligner/aligners/global_dynamic_programming_stack_aligner.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/aligner/aligners/utils.h"
#include "offbynull/aligner/aligners/concepts.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::aligners::global_dynamic_programming_stack_aligner::global_dynamic_programming_stack_aligner;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::aligner::aligners::utils::alignment_to_stacked_string;
    using offbynull::utils::is_debug_mode;

    TEST(OAAGlobalDynamicProgrammingStackAlignerTest, SanityTest) {
        global_dynamic_programming_stack_aligner<is_debug_mode()> aligner {};
        auto substitution_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::size_t>::create_substitution(1zu, 0zu) };
        auto gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::size_t>::create_gap(0zu) };
        std::string down { "panama" };
        std::string right { "banana" };
        const auto& [alignment, score] {
            aligner.align<10zu, 10zu>(down, right, substitution_scorer, gap_scorer)
        };
        EXPECT_EQ(4zu, score);
        std::cout << score << std::endl;
        std::cout << alignment_to_stacked_string<is_debug_mode()>(down, right, alignment) << std::endl;
    }

    TEST(OAAGlobalDynamicProgrammingStackAlignerTest, ParameterizationTest) {
        global_dynamic_programming_stack_aligner<is_debug_mode()> aligner {};

        std::string down { "panama" };
        std::string right { "banana" };

        auto align_ {
            [&]<typename N_INDEX, typename WEIGHT>() {
                auto substitution_scorer {
                    simple_scorer<is_debug_mode(), N_INDEX, char, char, WEIGHT>::create_substitution(
                        static_cast<WEIGHT>(1zu),
                        static_cast<WEIGHT>(0zu)
                    )
                };
                auto gap_scorer {
                    simple_scorer<is_debug_mode(), N_INDEX, char, char, WEIGHT>::create_gap(
                        static_cast<WEIGHT>(0zu)
                    )
                };
                const auto& [alignment, score] {
                    aligner.align_strict<N_INDEX, WEIGHT, 10zu, 10zu>(down, right, substitution_scorer, gap_scorer)
                };
                return score;
            }
        };

        // small index type
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::int8_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::int16_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::int32_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::int64_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::uint8_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::uint16_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::uint32_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::uint64_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::size_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::float32_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::uint8_t, std::float64_t>()));
        // large index type
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::int8_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::int16_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::int32_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::int64_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::uint8_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::uint16_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::uint32_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::uint64_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::size_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::float32_t>()));
        EXPECT_EQ(4zu, (align_.operator()<std::size_t, std::float64_t>()));
    }
}