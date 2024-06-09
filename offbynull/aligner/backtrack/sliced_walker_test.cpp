#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/backtrack/sliced_walker.h"
#include "gtest/gtest.h"
#include <format>
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrack::sliced_walker::sliced_forward_walker;
    using offbynull::aligner::backtrack::slot_container::slot;

    template<typename ND_, typename ED_, typename INDEX = unsigned int, bool error_check = true>
    auto create_vector(INDEX down_cnt, INDEX right_cnt) {
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

    TEST(SlicedWalkerTest, ForwardWalk) {
        using ND = std::tuple<>;
        using ED = std::float64_t;
        auto g { create_vector<ND, ED>(2u, 3u) };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        sliced_forward_walker<decltype(g), 2zu, unsigned int, std::float64_t, true> walker{g};  // 0,0
        EXPECT_FALSE(walker.next(g, [&g](const E& edge) { return g.get_edge_data(edge); }));  // 0,1
        EXPECT_FALSE(walker.next(g, [&g](const E& edge) { return g.get_edge_data(edge); }));  // 0,2
        EXPECT_FALSE(walker.next(g, [&g](const E& edge) { return g.get_edge_data(edge); }));  // 1,0
        EXPECT_FALSE(walker.next(g, [&g](const E& edge) { return g.get_edge_data(edge); }));  // 1,1
        EXPECT_FALSE(walker.next(g, [&g](const E& edge) { return g.get_edge_data(edge); }));  // 1,2
        EXPECT_TRUE(walker.next(g, [&g](const E& edge) { return g.get_edge_data(edge); }));  // 1,2

        RUN TEST;
        RUN TEST;
        RUN TEST;
        RUN TEST;
        RUN TEST;
    }
}
