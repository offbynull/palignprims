#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::backward_walker_iterator;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    TEST(PathContainerTest, BasicTest) {
        // TODO: Fix me.

        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        std::string seq1 { "abcdefg" };
        std::string seq2 { "abcZefg" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        path_container<true, decltype(g)> container { g };
        // auto e_ptr { container.initialize(-4) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
        // auto e_m1_ptr { container.push_prefix(e_ptr, -2) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
        // auto e_m1m1_ptr { container.push_prefix(e_m1_ptr, -1) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
        // auto e_m1p1_ptr { container.push_suffix(e_m1_ptr, -3) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
        // auto e_p1_ptr { container.push_suffix(e_ptr, -6) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_EQ(*it, *e_p1_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
        // auto e_p1m1_ptr { container.push_prefix(e_p1_ptr, -5) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_EQ(*it, *e_p1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_p1_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
        // auto e_p1p1_ptr { container.push_suffix(e_p1_ptr, -7) };
        // {
        //     auto r { container.to_range() };
        //     auto it { r.begin() };
        //     EXPECT_FALSE(it == r.end());
        //     EXPECT_EQ(*it, *e_m1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_m1p1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_ptr); ++it;
        //     EXPECT_EQ(*it, *e_p1m1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_p1_ptr); ++it;
        //     EXPECT_EQ(*it, *e_p1p1_ptr); ++it;
        //     EXPECT_TRUE(it == r.end());
        // }
    }
}