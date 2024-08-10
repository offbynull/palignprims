#include "offbynull/aligner/scorers/substitution_map_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::scorers::substitution_map_scorer::substitution_map_scorer;

    TEST(SubstitutionMapScorerTest, SanityTest) {
        substitution_map_scorer<true, char, char, int> scorer {
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
        EXPECT_EQ(1, (scorer(std::tuple<>{}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<>{}, { { a_ } }, { { b_ } })));
        EXPECT_EQ(-1, (scorer(std::tuple<>{}, { { a_ } }, { std::nullopt })));
        EXPECT_EQ(-1, (scorer(std::tuple<>{}, { std::nullopt }, { { b_ } })));
        EXPECT_EQ(0, (scorer(std::tuple<>{}, { std::nullopt }, { std::nullopt })));
    }
}