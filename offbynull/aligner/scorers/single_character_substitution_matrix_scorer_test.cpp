#include "offbynull/aligner/scorers/single_character_substitution_matrix_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>

namespace {
    using offbynull::aligner::scorers::single_character_substitution_matrix_scorer::single_character_substitution_matrix_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASSingleCharacterSubstitutionMatrixScorerTest, SanityTest) {
        single_character_substitution_matrix_scorer<is_debug_mode(), int, 4zu> scorer {
            R"(
    A   T   C   G
A   6  -7  -4  -3
T  -7   8  -6 -10
C  -4  -6   8   2
G  -3 -10   2   8
            )"
        };
        char a_ { 'A' };
        char c_ { 'C' };
        char t_ { 'T' };
        char g_ { 'G' };
        EXPECT_EQ(6, (scorer(std::tuple<> {}, { { a_ } }, { { a_ } })));
        EXPECT_EQ(-4, (scorer(std::tuple<> {}, { { a_ } }, { { c_ } })));
        EXPECT_EQ(-10, (scorer(std::tuple<> {}, { { g_ } }, { { t_ } })));
    }
}