#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrack/sliced_walker.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrack::sliced_walker::sliced_forward_walker;
    using offbynull::aligner::backtrack::sliced_walker::sliced_backward_walker;
    using offbynull::aligner::backtrack::sliced_walker::slot;

    template<typename ND_, typename ED_, typename INDEX = unsigned int, bool error_check = true>
    auto create_global(INDEX down_cnt, INDEX right_cnt) {
        return offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph<
            ND_,
            ED_,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ND_, INDEX>,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ED_, INDEX>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ND_, typename ED_, typename INDEX = unsigned int, bool error_check = true>
    auto create_local(INDEX down_cnt, INDEX right_cnt) {
        return offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph<
            ND_,
            ED_,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ND_, INDEX>,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ED_, INDEX>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(SlicedWalkerTest, ForwardWalkWithoutResidents) {
        using ND = std::tuple<>;
        using ED = std::float64_t;
        auto g { create_global<ND, ED>(4u, 4u) };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // indel update weight of -1
        g.update_edge_data(
            E { N { 0u, 0u }, N { 0u, 1u } },
            -1.0
        );
        // match update weight of -1
        for (const E& edge : g.get_edges()) {
            const auto& [n1, n2] { edge };
            const auto& [n1_down, n1_right] { n1 };
            const auto& [n2_down, n2_right] { n2 };
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                g.update_edge_data(edge, -1.0);
            }
        }

        // walk
        sliced_forward_walker<decltype(g), std::float64_t> walker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
    }

    TEST(SlicedWalkerTest, BackwardWalkWithoutResidents) {
        using ND = std::tuple<>;
        using ED = std::float64_t;
        auto g { create_global<ND, ED>(4u, 4u) };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // indel update weight of -1
        g.update_edge_data(
            E { N { 0u, 0u }, N { 0u, 1u } },
            -1.0
        );
        // match update weight of -1
        for (const E& edge : g.get_edges()) {
            const auto& [n1, n2] { edge };
            const auto& [n1_down, n1_right] { n1 };
            const auto& [n2_down, n2_right] { n2 };
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                g.update_edge_data(edge, -1.0);
            }
        }

        // walk
        sliced_backward_walker<decltype(g), std::float64_t> walker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -1.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -2.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
        EXPECT_TRUE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);

        EXPECT_TRUE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, -3.0);
    }

    TEST(SlicedWalkerTest, ForwardWalkWithResidents) {
        using ND = std::tuple<>;
        using ED = std::float64_t;
        auto g { create_local<ND, ED>(4u, 4u) };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // indel update weight of -1
        g.update_edge_data(
            E {
                offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::NORMAL,
                std::pair<N, N> { N { 0u, 0u }, N { 0u, 1u } }
            },
            -1.0
        );
        // match update weight of -1
        for (const E& edge : g.get_edges()) {
            if (edge.type == offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::FREE_RIDE) {
                continue;
            }
            const auto& [n1, n2] { edge.inner_edge };
            const auto& [n1_down, n1_right] { n1 };
            const auto& [n2_down, n2_right] { n2 };
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                g.update_edge_data(edge, -1.0);
            }
        }
        // middle match update weight of 1.0
        g.update_edge_data(
            E {
                offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::NORMAL,
                std::pair<N, N> { N { 1u, 1u }, N { 2u, 2u } }
            },
            0.5
        );

        // walk
        sliced_forward_walker<decltype(g), std::float64_t> walker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.5);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.5);

        EXPECT_TRUE(walker.next());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.5);
    }

    TEST(SlicedWalkerTest, BackwardWalkWithResidents) {
        using ND = std::tuple<>;
        using ED = std::float64_t;
        auto g { create_local<ND, ED>(4u, 4u) };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // indel update weight of -1
        g.update_edge_data(
            E {
                offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::NORMAL,
                std::pair<N, N> { N { 0u, 0u }, N { 0u, 1u } }
            },
            -1.0
        );
        // match update weight of -1
        for (const E& edge : g.get_edges()) {
            if (edge.type == offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::FREE_RIDE) {
                continue;
            }
            const auto& [n1, n2] { edge.inner_edge };
            const auto& [n1_down, n1_right] { n1 };
            const auto& [n2_down, n2_right] { n2 };
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                g.update_edge_data(edge, -1.0);
            }
        }
        // middle match update weight of 1.0
        g.update_edge_data(
            E {
                offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::NORMAL,
                std::pair<N, N> { N { 1u, 1u }, N { 2u, 2u } }
            },
            0.5
        );

        // walk
        sliced_backward_walker<decltype(g), std::float64_t> walker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 3u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 2u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.5);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 1u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);

        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 3u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 2u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_FALSE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 1u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.0);
        EXPECT_TRUE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.5);

        EXPECT_TRUE(walker.prev());
        EXPECT_EQ(walker.active_slot().node, (N { 0u, 0u }));
        EXPECT_EQ(walker.active_slot().backtracking_weight, 0.5);
    }
}
