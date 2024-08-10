#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>
#include <stdexcept>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    TEST(ForwardWalkerTest, ForwardWalkWithoutResidents) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        std::string seq1 { "abc" };
        std::string seq2 { "azc" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using ND = typename decltype(g)::ND;
        using ED = typename decltype(g)::ED;
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 0u) };
            EXPECT_EQ(walker.find(N { 0zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 1zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 0zu, 2zu }).backtracking_weight, -2.0);
            EXPECT_EQ(walker.find(N { 0zu, 3zu }).backtracking_weight, -3.0);
            EXPECT_THROW(walker.find(N { 1zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 3zu }).backtracking_weight, std::runtime_error);
        }

        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 1u) };
            EXPECT_EQ(walker.find(N { 0zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 1zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 0zu, 2zu }).backtracking_weight, -2.0);
            EXPECT_EQ(walker.find(N { 0zu, 3zu }).backtracking_weight, -3.0);
            EXPECT_EQ(walker.find(N { 1zu, 0zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 1zu, 1zu }).backtracking_weight, 1.0);
            EXPECT_EQ(walker.find(N { 1zu, 2zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 3zu }).backtracking_weight, -1.0);
            EXPECT_THROW(walker.find(N { 2zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 3zu }).backtracking_weight, std::runtime_error);
        }

        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 2u) };
            EXPECT_THROW(walker.find(N { 0zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 1zu, 0zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 1zu, 1zu }).backtracking_weight, 1.0);
            EXPECT_EQ(walker.find(N { 1zu, 2zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 3zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 2zu, 0zu }).backtracking_weight, -2.0);
            EXPECT_EQ(walker.find(N { 2zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 2zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 3zu }).backtracking_weight, -1.0);
            EXPECT_THROW(walker.find(N { 3zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 3zu }).backtracking_weight, std::runtime_error);
        }

        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 3u) };
            EXPECT_THROW(walker.find(N { 0zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 2zu, 0zu }).backtracking_weight, -2.0);
            EXPECT_EQ(walker.find(N { 2zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 2zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 3zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 3zu, 0zu }).backtracking_weight, -3.0);
            EXPECT_EQ(walker.find(N { 3zu, 1zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 3zu, 2zu }).backtracking_weight, -1.0);
            EXPECT_EQ(walker.find(N { 3zu, 3zu }).backtracking_weight, 1.0);
        }
    }

    TEST(ForwardWalkerTest, ForwardWalkWithResidents) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaa" };
        std::string seq2 { "zaz" };
        pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using ND = typename decltype(g)::ND;
        using ED = typename decltype(g)::ED;
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 0u) };
            EXPECT_EQ(walker.find(N { 0zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 2zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 3zu }).backtracking_weight, 0.0);
            EXPECT_THROW(walker.find(N { 1zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 3zu, 3zu }).backtracking_weight, 0.0);
        }

        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 1u) };
            EXPECT_EQ(walker.find(N { 0zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 2zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 0zu, 3zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 2zu }).backtracking_weight, 1.0);
            EXPECT_EQ(walker.find(N { 1zu, 3zu }).backtracking_weight, 0.0);
            EXPECT_THROW(walker.find(N { 2zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 2zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 3zu, 3zu }).backtracking_weight, 1.0);
        }

        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 2u) };
            EXPECT_EQ(walker.find(N { 0zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_THROW(walker.find(N { 0zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 1zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 1zu, 2zu }).backtracking_weight, 1.0);
            EXPECT_EQ(walker.find(N { 1zu, 3zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 2zu, 2zu }).backtracking_weight, 1.0);
            EXPECT_EQ(walker.find(N { 2zu, 3zu }).backtracking_weight, 0.0);
            EXPECT_THROW(walker.find(N { 3zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 3zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 3zu, 3zu }).backtracking_weight, 1.0);
        }

        {
            forward_walker<true, decltype(g)> walker{ forward_walker<true, decltype(g)>::create_and_initialize(g, 3u) };
            EXPECT_EQ(walker.find(N { 0zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_THROW(walker.find(N { 0zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 0zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 0zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 1zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 2zu }).backtracking_weight, std::runtime_error);
            EXPECT_THROW(walker.find(N { 1zu, 3zu }).backtracking_weight, std::runtime_error);
            EXPECT_EQ(walker.find(N { 3zu, 0zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 3zu, 1zu }).backtracking_weight, 0.0);
            EXPECT_EQ(walker.find(N { 3zu, 2zu }).backtracking_weight, 1.0);
            EXPECT_EQ(walker.find(N { 3zu, 3zu }).backtracking_weight, 1.0);
        }
    }
}
