#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/aligner/scorers/consumption_gating_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <optional>

namespace {
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::aligner::scorers::consumption_gating_scorer::consumption_gating_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OASConsumptionGatingScorerTest, SanityTest) {
        simple_scorer<is_debug_mode(), std::size_t, char, char, int> backing_scorer { 1, -1, -1, -1, -1 };
        consumption_gating_scorer<is_debug_mode(), char, char, decltype(backing_scorer)> scorer { 10, 9, backing_scorer };
        char a_ { 'a' };

        auto weight { scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } }) };
        auto weight_og { scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } }) };
        EXPECT_EQ(1, weight.real_weight);
        EXPECT_EQ(1, weight.down_consumed);
        EXPECT_EQ(1, weight.right_consumed);
        EXPECT_FALSE(weight < weight_og);
        weight = weight + scorer({ { 0zu, { a_ } } }, std::nullopt);
        weight = weight + scorer(std::nullopt, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, std::nullopt);
        weight = weight + scorer(std::nullopt, std::nullopt);
        EXPECT_EQ(-3, weight.real_weight);
        EXPECT_EQ(3, weight.down_consumed);
        EXPECT_EQ(2, weight.right_consumed);
        EXPECT_TRUE(weight < weight_og);
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        weight = weight + scorer({ { 0zu, { a_ } } }, { { 0zu, { a_ } } });
        EXPECT_EQ(4, weight.real_weight);
        EXPECT_EQ(10, weight.down_consumed);
        EXPECT_EQ(9, weight.right_consumed);
        EXPECT_FALSE(weight < weight_og);
        {
            auto next_weight { weight + scorer({ { 0zu, { a_ } } }, std::nullopt) };
            EXPECT_EQ(3, next_weight.real_weight);
            EXPECT_EQ(11, next_weight.down_consumed);
            EXPECT_EQ(9, next_weight.right_consumed);
            EXPECT_TRUE(next_weight < weight_og);  // DOWN EXCEEDED, meaning even though this should be true, it'll return false
        }
        {
            auto next_weight { weight + scorer(std::nullopt, { { 0zu, { a_ } } }) };
            EXPECT_EQ(3, next_weight.real_weight);
            EXPECT_EQ(10, next_weight.down_consumed);
            EXPECT_EQ(10, next_weight.right_consumed);
            EXPECT_TRUE(next_weight < weight_og);  // RIGHT EXCEEDED, meaning even though this should be true, it'll return false
        }
    }
}