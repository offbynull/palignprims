#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::internal_range;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker::slot;

    TEST(PathContainerTest, BasicTest) {
        path_container<int, int> container { 50zu };
        auto e_ptr { container.initialize(-4) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
        auto e_m1_ptr { container.push_prefix(e_ptr, -2) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_m1_ptr); ++it;
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
        auto e_m1m1_ptr { container.push_prefix(e_m1_ptr, -1) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1_ptr); ++it;
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
        auto e_m1p1_ptr { container.push_suffix(e_m1_ptr, -3) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
        auto e_p1_ptr { container.push_suffix(e_ptr, -6) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_EQ(*it, *e_p1_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
        auto e_p1m1_ptr { container.push_prefix(e_p1_ptr, -5) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_EQ(*it, *e_p1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_p1_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
        auto e_p1p1_ptr { container.push_suffix(e_p1_ptr, -7) };
        {
            auto r { container.to_range() };
            auto it { r.begin() };
            EXPECT_FALSE(it == r.end());
            EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1_ptr); ++it;
            EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
            EXPECT_EQ(*it, *e_ptr); ++it;
            EXPECT_EQ(*it, *e_p1m1_ptr); ++it;
            EXPECT_EQ(*it, *e_p1_ptr); ++it;
            EXPECT_EQ(*it, *e_p1p1_ptr); ++it;
            EXPECT_TRUE(it == r.end());
        }
    }
}