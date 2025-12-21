#include <cstddef>
#include <stdfloat>
#include <string>
#include <vector>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::generic_row_nodes;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node_layer;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_vector;
    using offbynull::utils::is_debug_mode;

    TEST(OAGSliceablePairwiseAlignmentGraphTest, RowWalk) {
        auto substitution_scorer {
            simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64)
        };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_extended_gap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(initial_gap_scorer),
            decltype(extended_gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_EQ(
            (copy_to_vector(generic_row_nodes<is_debug_mode()>(g, 0u))),
            (std::vector<N> {
                N { node_layer::DIAGONAL, 0zu, 0zu },
                N { node_layer::RIGHT, 0zu, 1zu },
                N { node_layer::DIAGONAL, 0zu, 1zu },
                N { node_layer::RIGHT, 0zu, 2zu },
                N { node_layer::DIAGONAL, 0zu, 2zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(generic_row_nodes<is_debug_mode()>(g, 1u))),
            (std::vector<N> {
                N { node_layer::DOWN, 1zu, 0zu },
                N { node_layer::DIAGONAL, 1zu, 0zu },
                N { node_layer::DOWN, 1zu, 1zu },
                N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 2zu }
            })
        );
    }

    TEST(OAGSliceablePairwiseAlignmentGraphTest, RowWalkPartial) {
        auto substitution_scorer {
            simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64)
        };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "accd" };
        std::string seq2 { "accd" };
        pairwise_extended_gap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(initial_gap_scorer),
            decltype(extended_gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        // If you're confused about any of the tests below, use middle_sliceable_alignment_graph to isolate to the root/leaf nodes
        // specified and dump it out using pairwise_graph_to_graphviz().

        EXPECT_EQ(
            (copy_to_vector(
                generic_row_nodes<is_debug_mode()>(g, 0u, N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))
            ),
            (std::vector<N> {
                N { node_layer::DIAGONAL, 0zu, 0zu },
                N { node_layer::RIGHT, 0zu, 1zu },
                N { node_layer::DIAGONAL, 0zu, 1zu },
                N { node_layer::RIGHT, 0zu, 2zu },
                N { node_layer::DIAGONAL, 0zu, 2zu },
                N { node_layer::RIGHT, 0zu, 3zu },
                N { node_layer::DIAGONAL, 0zu, 3zu },
                N { node_layer::RIGHT, 0zu, 4zu },
                N { node_layer::DIAGONAL, 0zu, 4zu },
            })
        );
        EXPECT_EQ(
            (copy_to_vector(
                generic_row_nodes<is_debug_mode()>(g, 4u, N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))
            ),
            (std::vector<N> {
                N { node_layer::DOWN, 4zu, 0zu },
                N { node_layer::DIAGONAL, 4zu, 0zu },
                N { node_layer::DOWN, 4zu, 1zu },
                N { node_layer::RIGHT, 4zu, 1zu },
                N { node_layer::DIAGONAL, 4zu, 1zu },
                N { node_layer::DOWN, 4zu, 2zu },
                N { node_layer::RIGHT, 4zu, 2zu },
                N { node_layer::DIAGONAL, 4zu, 2zu },
                N { node_layer::DOWN, 4zu, 3zu },
                N { node_layer::RIGHT, 4zu, 3zu },
                N { node_layer::DIAGONAL, 4zu, 3zu },
                N { node_layer::DOWN, 4zu, 4zu },
                N { node_layer::RIGHT, 4zu, 4zu },
                N { node_layer::DIAGONAL, 4zu, 4zu }
            })
        );

        EXPECT_EQ(
            (copy_to_vector(
                generic_row_nodes<is_debug_mode()>(g, 1u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))
            ),
            (std::vector<N> {
                // N { node_layer::DOWN, 1zu, 1zu },
                // N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                // N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 2zu },
                // N { node_layer::DOWN, 1zu, 3zu },
                N { node_layer::RIGHT, 1zu, 3zu },
                N { node_layer::DIAGONAL, 1zu, 3zu },
                // N { node_layer::DOWN, 1zu, 4zu },
                N { node_layer::RIGHT, 1zu, 4zu },
                N { node_layer::DIAGONAL, 1zu, 4zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(
                generic_row_nodes<is_debug_mode()>(g, 2u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))
            ),
            (std::vector<N> {
                N { node_layer::DOWN, 2zu, 1zu },
                // N { node_layer::RIGHT, 2zu, 1zu },
                N { node_layer::DIAGONAL, 2zu, 1zu },
                N { node_layer::DOWN, 2zu, 2zu },
                N { node_layer::RIGHT, 2zu, 2zu },
                N { node_layer::DIAGONAL, 2zu, 2zu },
                N { node_layer::DOWN, 2zu, 3zu },
                N { node_layer::RIGHT, 2zu, 3zu },
                N { node_layer::DIAGONAL, 2zu, 3zu },
                N { node_layer::DOWN, 2zu, 4zu },
                N { node_layer::RIGHT, 2zu, 4zu },
                N { node_layer::DIAGONAL, 2zu, 4zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(
                generic_row_nodes<is_debug_mode()>(g, 4u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))
            ),
            (std::vector<N> {
                N { node_layer::DOWN, 4zu, 1zu },
                // N { node_layer::RIGHT, 4zu, 1zu },
                N { node_layer::DIAGONAL, 4zu, 1zu },
                N { node_layer::DOWN, 4zu, 2zu },
                N { node_layer::RIGHT, 4zu, 2zu },
                N { node_layer::DIAGONAL, 4zu, 2zu },
                N { node_layer::DOWN, 4zu, 3zu },
                N { node_layer::RIGHT, 4zu, 3zu },
                N { node_layer::DIAGONAL, 4zu, 3zu },
                N { node_layer::DOWN, 4zu, 4zu },
                N { node_layer::RIGHT, 4zu, 4zu },
                N { node_layer::DIAGONAL, 4zu, 4zu }
            })
        );

        EXPECT_EQ(
            (copy_to_vector(
                generic_row_nodes<is_debug_mode()>(g, 1u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::RIGHT, 1zu, 4zu }))
            ),
            (std::vector<N> {
                // N { node_layer::DOWN, 1zu, 1zu },
                // N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                // N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 2zu },
                // N { node_layer::DOWN, 1zu, 3zu },
                N { node_layer::RIGHT, 1zu, 3zu },
                N { node_layer::DIAGONAL, 1zu, 3zu },
                // N { node_layer::DOWN, 1zu, 4zu },
                N { node_layer::RIGHT, 1zu, 4zu },
                // N { node_layer::DIAGONAL, 1zu, 4zu }
            })
        );
    }
}