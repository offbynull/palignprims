#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrack/sliced_backtrack.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrack::sliced_backtrack::sliced_backtracker;
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

    TEST(SlicedBacktrackTest, ForwardWalkWithoutResidents) {
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
            const auto& [n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_grid_down, n2_grid_right] { n2 };
            if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                g.update_edge_data(edge, -1.0);
            }
        }

        // walk
        sliced_backtracker<decltype(g), std::float64_t> backtracker{
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        };
        backtracker.walk();
    }
}
