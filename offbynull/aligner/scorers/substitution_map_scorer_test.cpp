#include "offbynull/aligner/scorers/substitution_map_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <optional>
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::substitution_map_scorer::substitution_map_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASSubstitutionMapScorerTest, SanityTest) {
        substitution_map_scorer<is_debug_mode(), std::size_t, char, char, int> scorer {
            {
                { { 'a', 'a' }, 1 },
                { { 'a', 'b' }, 0 },
                { { 'a', std::nullopt }, -1 },
                { { 'b', 'a' }, 0 },
                { { 'b', 'b' }, 1 },
                { { 'b', std::nullopt }, -1 },
                { { std::nullopt, 'a' }, -1 },
                { { std::nullopt, 'b' }, -1 },
                { { std::nullopt, std::nullopt }, 0 },
            }
        };
        char a_ { 'a' };
        char b_ { 'b' };
        EXPECT_EQ(1, (scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(0, (scorer({ { 0zu, { a_ } } }, { { 0zu, { b_ } } })));
        EXPECT_EQ(-1, (scorer({ { 0zu, { a_ } } }, { std::nullopt })));
        EXPECT_EQ(-1, (scorer({ std::nullopt }, { { 0zu, { b_ } } })));
        EXPECT_EQ(0, (scorer({ std::nullopt }, { std::nullopt })));
    }
}