#include "offbynull/aligner/scorers/pam_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <tuple>
#include <cstddef>

namespace {
    using offbynull::aligner::scorers::pam_scorer::distance;
    using offbynull::aligner::scorers::pam_scorer::pam_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASPamScorerTest, SanityTest30) {
        pam_scorer<is_debug_mode(), distance::_30, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(6, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-6, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(10, (scorer(std::tuple<> {}, { { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-6, (scorer(std::tuple<> {}, { { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }

    TEST(OASPamScorerTest, SanityTest70) {
        pam_scorer<is_debug_mode(), distance::_70, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(5, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-4, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(9, (scorer(std::tuple<> {}, { { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-4, (scorer(std::tuple<> {}, { { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }

    TEST(OASPamScorerTest, SanityTest250) {
        pam_scorer<is_debug_mode(), distance::_250, std::size_t, int> scorer {};
        char a_ { 'A' };
        char c_ { 'C' };
        EXPECT_EQ(2, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { a_ } } })));
        EXPECT_EQ(-2, (scorer(std::tuple<> {}, { { 0zu, { a_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(12, (scorer(std::tuple<> {}, { { 0zu, { c_ } } }, { { 0zu, { c_ } } })));
        EXPECT_EQ(-2, (scorer(std::tuple<> {}, { { 0zu, { c_ } } }, { { 0zu, { a_ } } })));
    }
}