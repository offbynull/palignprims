#include <cstddef>
#include <stdfloat>
#include <string>
#include <vector>
#include <type_traits>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graph::multithreaded_sliceable_pairwise_alignment_graph::axis;
    using offbynull::aligner::graph::multithreaded_sliceable_pairwise_alignment_graph::generic_segmented_diagonal_nodes;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node_layer;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_vector;
    using offbynull::utils::is_debug_mode;


    auto to_vector_of_vector(auto&& r1) {
        using E = std::remove_cvref_t<decltype(*(*r1.begin()).begin())>;
        std::vector<std::vector<E>> v1 {};
        for (auto&& inner_r1 : r1) {
            std::vector<E> inner_v1 {};
            for (const E& e : inner_r1) {
                inner_v1.push_back(e);
            }
            v1.push_back(inner_v1);
        }
        return v1;
    }


    TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkUnsegmented) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride() };
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
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 0u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DIAGONAL, 0zu, 0zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 1u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu, 0zu },
                    N { node_layer::DIAGONAL, 1zu, 0zu },
                    N { node_layer::RIGHT, 0zu, 1zu },
                    N { node_layer::DIAGONAL, 0zu, 1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 0u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu, 0zu },
                    N { node_layer::DIAGONAL, 1zu, 0zu },
                    N { node_layer::RIGHT, 0zu, 1zu },
                    N { node_layer::DIAGONAL, 0zu, 1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 1u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu, 1zu },
                    N { node_layer::RIGHT, 1zu, 1zu },
                    N { node_layer::DIAGONAL, 1zu, 1zu },
                    N { node_layer::RIGHT, 0zu, 2zu },
                    N { node_layer::DIAGONAL, 0zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 2u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu, 2zu },
                    N { node_layer::RIGHT, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 1zu, 2zu }
                }
            })
        );
    }

    TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkUnsegmentedDownShorter) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "01" };
        std::string seq2 { "0123" };
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
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 0u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DIAGONAL, 0zu, 0zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 1u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu, 0zu },
                    N { node_layer::DIAGONAL, 1zu, 0zu },
                    N { node_layer::RIGHT, 0zu, 1zu },
                    N { node_layer::DIAGONAL, 0zu, 1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 2u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 0zu },
                    N { node_layer::DIAGONAL, 2zu, 0zu },
                    N { node_layer::DOWN, 1zu, 1zu },
                    N { node_layer::RIGHT, 1zu, 1zu },
                    N { node_layer::DIAGONAL, 1zu, 1zu },
                    N { node_layer::RIGHT, 0zu, 2zu },
                    N { node_layer::DIAGONAL, 0zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 0u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 0zu },
                    N { node_layer::DIAGONAL, 2zu, 0zu },
                    N { node_layer::DOWN, 1zu, 1zu },
                    N { node_layer::RIGHT, 1zu, 1zu },
                    N { node_layer::DIAGONAL, 1zu, 1zu },
                    N { node_layer::RIGHT, 0zu, 2zu },
                    N { node_layer::DIAGONAL, 0zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 1u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 1zu },
                    N { node_layer::RIGHT, 2zu, 1zu },
                    N { node_layer::DIAGONAL, 2zu, 1zu },
                    N { node_layer::DOWN, 1zu, 2zu },
                    N { node_layer::RIGHT, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::RIGHT, 0zu, 3zu },
                    N { node_layer::DIAGONAL, 0zu, 3zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 2u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 2zu },
                    N { node_layer::RIGHT, 2zu, 2zu },
                    N { node_layer::DIAGONAL, 2zu, 2zu },
                    N { node_layer::DOWN, 1zu, 3zu },
                    N { node_layer::RIGHT, 1zu, 3zu },
                    N { node_layer::DIAGONAL, 1zu, 3zu },
                    N { node_layer::RIGHT, 0zu, 4zu },
                    N { node_layer::DIAGONAL, 0zu, 4zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 3u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 3zu },
                    N { node_layer::RIGHT, 2zu, 3zu },
                    N { node_layer::DIAGONAL, 2zu, 3zu },
                    N { node_layer::DOWN, 1zu, 4zu },
                    N { node_layer::RIGHT, 1zu, 4zu },
                    N { node_layer::DIAGONAL, 1zu, 4zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 4u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 4zu },
                    N { node_layer::RIGHT, 2zu, 4zu },
                    N { node_layer::DIAGONAL, 2zu, 4zu }
                }
            })
        );
    }

    TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkUnsegmentedRightShorter) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "0123" };
        std::string seq2 { "01" };
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
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 0u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DIAGONAL, 0zu, 0zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 1u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu, 0zu },
                    N { node_layer::DIAGONAL, 1zu, 0zu },
                    N { node_layer::RIGHT, 0zu, 1zu },
                    N { node_layer::DIAGONAL, 0zu, 1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 2u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 2zu, 0zu },
                    N { node_layer::DIAGONAL, 2zu, 0zu },
                    N { node_layer::DOWN, 1zu, 1zu },
                    N { node_layer::RIGHT, 1zu, 1zu },
                    N { node_layer::DIAGONAL, 1zu, 1zu },
                    N { node_layer::RIGHT, 0zu, 2zu },
                    N { node_layer::DIAGONAL, 0zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 3u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 3zu, 0zu },
                    N { node_layer::DIAGONAL, 3zu, 0zu },
                    N { node_layer::DOWN, 2zu, 1zu },
                    N { node_layer::RIGHT, 2zu, 1zu },
                    N { node_layer::DIAGONAL, 2zu, 1zu },
                    N { node_layer::DOWN, 1zu, 2zu },
                    N { node_layer::RIGHT, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 1zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::DOWN_FROM_TOP_LEFT, 4u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 4zu, 0zu },
                    N { node_layer::DIAGONAL, 4zu, 0zu },
                    N { node_layer::DOWN, 3zu, 1zu },
                    N { node_layer::RIGHT, 3zu, 1zu },
                    N { node_layer::DIAGONAL, 3zu, 1zu },
                    N { node_layer::DOWN, 2zu, 2zu },
                    N { node_layer::RIGHT, 2zu, 2zu },
                    N { node_layer::DIAGONAL, 2zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 0u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 4zu, 0zu },
                    N { node_layer::DIAGONAL, 4zu, 0zu },
                    N { node_layer::DOWN, 3zu, 1zu },
                    N { node_layer::RIGHT, 3zu, 1zu },
                    N { node_layer::DIAGONAL, 3zu, 1zu },
                    N { node_layer::DOWN, 2zu, 2zu },
                    N { node_layer::RIGHT, 2zu, 2zu },
                    N { node_layer::DIAGONAL, 2zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 1u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 4zu, 1zu },
                    N { node_layer::RIGHT, 4zu, 1zu },
                    N { node_layer::DIAGONAL, 4zu, 1zu },
                    N { node_layer::DOWN, 3zu, 2zu },
                    N { node_layer::RIGHT, 3zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(generic_segmented_diagonal_nodes<is_debug_mode()>(g, axis::RIGHT_FROM_BOTTOM_LEFT, 2u, 1zu))),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 4zu, 2zu },
                    N { node_layer::RIGHT, 4zu, 2zu },
                    N { node_layer::DIAGONAL, 4zu, 2zu }
                }
            })
        );
    }

    TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkWithCustomRootLeafUnsegmentedDownShorter) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "x01xx" };
        std::string seq2 { "yy0123yyy" };
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
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    0u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+0zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    1u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+0zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    2u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    0u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    1u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+1zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+3zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    2u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+2zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+4zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    3u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+3zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+4zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    4u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 3zu, 6zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+4zu }
                }
            })
        );
    }

    TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkWithCustomRootLeafUnsegmentedRightShorter) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "x0123xx" };
        std::string seq2 { "yy01yyy" };
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
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    0u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+0zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    1u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+0zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    2u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    3u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+3zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+2zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+1zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    4u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+4zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+3zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+1zu },
                    N { node_layer::DOWN, 1zu+2zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    0u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+4zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+3zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+1zu },
                    N { node_layer::DOWN, 1zu+2zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    1u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+4zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+4zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+1zu },
                    N { node_layer::DOWN, 1zu+3zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    2u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 5zu, 4zu },
                    1zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+4zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+4zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+2zu }
                }
            })
        );
    }

    TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkWithCustomRootLeafSegmentedRightShorter) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride() };
        std::string seq1 { "x123412341234123xx" };
        std::string seq2 { "yy12341234123412y" };
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
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    0u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+0zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    1u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+0zu },
                },
                {
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+1zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    2u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+0zu }
                },
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+1zu }
                },
                {
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+2zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    3u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+3zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+0zu }
                },
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+1zu }
                },
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+2zu }
                },
                {
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+3zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    4u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+4zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+3zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+1zu }
                },
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+2zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+3zu }
                },
                {
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+4zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    5u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+5zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+5zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+4zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+4zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+1zu }
                },
                {
                    N { node_layer::DOWN, 1zu+3zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+2zu },
                    N { node_layer::DOWN, 1zu+2zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+3zu }
                },
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+5zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+5zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    6u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+6zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+6zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+5zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+5zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+5zu, 2zu+1zu }
                },
                {
                    N { node_layer::DOWN, 1zu+4zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+4zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+2zu },
                    N { node_layer::DOWN, 1zu+3zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+3zu }
                },
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+4zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+5zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+5zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+5zu }
                },
                {
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+6zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+6zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    7u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+7zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+7zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+6zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+6zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+6zu, 2zu+1zu }
                },
                {
                    N { node_layer::DOWN, 1zu+5zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+5zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+5zu, 2zu+2zu },
                    N { node_layer::DOWN, 1zu+4zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+4zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+3zu }
                },
                {
                    N { node_layer::DOWN, 1zu+3zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+4zu },
                    N { node_layer::DOWN, 1zu+2zu, 2zu+5zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+5zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+5zu }
                },
                {
                    N { node_layer::DOWN, 1zu+1zu, 2zu+6zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+6zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+6zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+7zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+7zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::DOWN_FROM_TOP_LEFT,
                    8u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+8zu, 2zu+0zu },
                    N { node_layer::DIAGONAL, 1zu+8zu, 2zu+0zu },
                    N { node_layer::DOWN, 1zu+7zu, 2zu+1zu },
                    N { node_layer::RIGHT, 1zu+7zu, 2zu+1zu },
                    N { node_layer::DIAGONAL, 1zu+7zu, 2zu+1zu },
                    N { node_layer::DOWN, 1zu+6zu, 2zu+2zu },
                    N { node_layer::RIGHT, 1zu+6zu, 2zu+2zu },
                    N { node_layer::DIAGONAL, 1zu+6zu, 2zu+2zu },
                },
                {
                    N { node_layer::DOWN, 1zu+5zu, 2zu+3zu },
                    N { node_layer::RIGHT, 1zu+5zu, 2zu+3zu },
                    N { node_layer::DIAGONAL, 1zu+5zu, 2zu+3zu },
                    N { node_layer::DOWN, 1zu+4zu, 2zu+4zu },
                    N { node_layer::RIGHT, 1zu+4zu, 2zu+4zu },
                    N { node_layer::DIAGONAL, 1zu+4zu, 2zu+4zu },
                    N { node_layer::DOWN, 1zu+3zu, 2zu+5zu },
                    N { node_layer::RIGHT, 1zu+3zu, 2zu+5zu },
                    N { node_layer::DIAGONAL, 1zu+3zu, 2zu+5zu }
                },
                {
                    N { node_layer::DOWN, 1zu+2zu, 2zu+6zu },
                    N { node_layer::RIGHT, 1zu+2zu, 2zu+6zu },
                    N { node_layer::DIAGONAL, 1zu+2zu, 2zu+6zu },
                    N { node_layer::DOWN, 1zu+1zu, 2zu+7zu },
                    N { node_layer::RIGHT, 1zu+1zu, 2zu+7zu },
                    N { node_layer::DIAGONAL, 1zu+1zu, 2zu+7zu },
                    N { node_layer::RIGHT, 1zu+0zu, 2zu+8zu },
                    N { node_layer::DIAGONAL, 1zu+0zu, 2zu+8zu }
                }
            })
        );
        // ... REMOVED MIDDLE ...
        // ... REMOVED MIDDLE ...
        // ... REMOVED MIDDLE ...
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    10u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+15zu, 2zu+10zu },
                    N { node_layer::RIGHT, 1zu+15zu, 2zu+10zu },
                    N { node_layer::DIAGONAL, 1zu+15zu, 2zu+10zu },
                    N { node_layer::DOWN, 1zu+14zu, 2zu+11zu },
                    N { node_layer::RIGHT, 1zu+14zu, 2zu+11zu },
                    N { node_layer::DIAGONAL, 1zu+14zu, 2zu+11zu }
                },
                {
                    N { node_layer::DOWN, 1zu+13zu, 2zu+12zu },
                    N { node_layer::RIGHT, 1zu+13zu, 2zu+12zu },
                    N { node_layer::DIAGONAL, 1zu+13zu, 2zu+12zu },
                    N { node_layer::DOWN, 1zu+12zu, 2zu+13zu },
                    N { node_layer::RIGHT, 1zu+12zu, 2zu+13zu },
                    N { node_layer::DIAGONAL, 1zu+12zu, 2zu+13zu }
                },
                {
                    N { node_layer::DOWN, 1zu+11zu, 2zu+14zu },
                    N { node_layer::RIGHT, 1zu+11zu, 2zu+14zu },
                    N { node_layer::DIAGONAL, 1zu+11zu, 2zu+14zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    11u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+15zu, 2zu+11zu },
                    N { node_layer::RIGHT, 1zu+15zu, 2zu+11zu },
                    N { node_layer::DIAGONAL, 1zu+15zu, 2zu+11zu }
                },
                {
                    N { node_layer::DOWN, 1zu+14zu, 2zu+12zu },
                    N { node_layer::RIGHT, 1zu+14zu, 2zu+12zu },
                    N { node_layer::DIAGONAL, 1zu+14zu, 2zu+12zu }
                },
                {
                    N { node_layer::DOWN, 1zu+13zu, 2zu+13zu },
                    N { node_layer::RIGHT, 1zu+13zu, 2zu+13zu },
                    N { node_layer::DIAGONAL, 1zu+13zu, 2zu+13zu }
                },
                {
                    N { node_layer::DOWN, 1zu+12zu, 2zu+14zu },
                    N { node_layer::RIGHT, 1zu+12zu, 2zu+14zu },
                    N { node_layer::DIAGONAL, 1zu+12zu, 2zu+14zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    12u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+15zu, 2zu+12zu },
                    N { node_layer::RIGHT, 1zu+15zu, 2zu+12zu },
                    N { node_layer::DIAGONAL, 1zu+15zu, 2zu+12zu }
                },
                {
                    N { node_layer::DOWN, 1zu+14zu, 2zu+13zu },
                    N { node_layer::RIGHT, 1zu+14zu, 2zu+13zu },
                    N { node_layer::DIAGONAL, 1zu+14zu, 2zu+13zu }
                },
                {
                    N { node_layer::DOWN, 1zu+13zu, 2zu+14zu },
                    N { node_layer::RIGHT, 1zu+13zu, 2zu+14zu },
                    N { node_layer::DIAGONAL, 1zu+13zu, 2zu+14zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    13u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+15zu, 2zu+13zu },
                    N { node_layer::RIGHT, 1zu+15zu, 2zu+13zu },
                    N { node_layer::DIAGONAL, 1zu+15zu, 2zu+13zu }
                },
                {
                    N { node_layer::DOWN, 1zu+14zu, 2zu+14zu },
                    N { node_layer::RIGHT, 1zu+14zu, 2zu+14zu },
                    N { node_layer::DIAGONAL, 1zu+14zu, 2zu+14zu }
                }
            })
        );
        EXPECT_EQ(
            (to_vector_of_vector(
                generic_segmented_diagonal_nodes<is_debug_mode()>(
                    g,
                    axis::RIGHT_FROM_BOTTOM_LEFT,
                    14u,
                    N { node_layer::DIAGONAL, 1zu, 2zu },
                    N { node_layer::DIAGONAL, 16zu, 16zu },
                    4zu
                )
            )),
            (std::vector<std::vector<N>> {
                {
                    N { node_layer::DOWN, 1zu+15zu, 2zu+14zu },
                    N { node_layer::RIGHT, 1zu+15zu, 2zu+14zu },
                    N { node_layer::DIAGONAL, 1zu+15zu, 2zu+14zu }
                }
            })
        );
    }

    // TEST(OAGMultithreadedSliceablePairwiseAlignmentGraphTest, DiagionalWalkWithCustomRootLeafSegmentedDownShorter) {
    //     IMPLEMENT;
    // }
}