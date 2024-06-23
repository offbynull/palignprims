#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrack/sliced_walker.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrack::sliced_walker::sliced_walker;
    using offbynull::aligner::backtrack::sliced_walker::slot;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;

    TEST(SlicedWalkerTest, ForwardWalkWithoutResidents) {
        auto match_lookup {
            [](
                const auto& edge,
                const char& down_elem,
                const char& right_elem
            ) -> std::float64_t {
                if (down_elem == right_elem) {
                    return 1.0f64;
                } else {
                    return -1.0f64;
                }
            }
        };
        auto indel_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return -1.0f64;
            }
        };
        std::string seq1 { "abc" };
        std::string seq2 { "azc" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup
        };

        using ND = typename decltype(g)::ND;
        using ED = typename decltype(g)::ED;
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        sliced_walker<decltype(g), std::float64_t> walker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);

        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);
    }

    TEST(SlicedWalkerTest, ForwardWalkWithResidents) {
        auto match_lookup {
            [](
                const auto& edge,
                const char& down_elem,
                const char& right_elem
            ) -> std::float64_t {
                if (down_elem == right_elem) {
                    return 1.0f64;
                } else {
                    return -1.0f64;
                }
            }
        };
        auto indel_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return -1.0f64;
            }
        };
        auto freeride_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };

        std::string seq1 { "aaa" };
        std::string seq2 { "zaz" };
        pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };

        using ND = typename decltype(g)::ND;
        using ED = typename decltype(g)::ED;
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        sliced_walker<decltype(g), std::float64_t> walker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 0zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 1zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 2zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);
        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);

        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3zu, 3zu }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 1.0);
    }
}
