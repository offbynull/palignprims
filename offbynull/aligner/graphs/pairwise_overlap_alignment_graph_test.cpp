#include <cstddef>
#include <stdfloat>
#include <string>
#include <set>
#include <vector>
#include <type_traits>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::create_pairwise_overlap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge_type;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_vector;
    using offbynull::utils::copy_to_set;
    using offbynull::utils::is_debug_mode;

    auto substitution_scorer {
        simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64)
    };
    auto gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(0.0f64) };
    auto freeride_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_freeride() };

    TEST(OAGPairwiseOverlapAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            std::string,
            std::string,
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        >;
        static_assert(offbynull::aligner::graph::graph::graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph<G>);
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, ListNodes) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        auto n = g.get_nodes();
        EXPECT_EQ(
            copy_to_set(n),
            (std::set<N> {
                N { 0zu, 0zu }, N { 0zu, 1zu }, N { 0zu, 2zu },
                N { 1zu, 0zu }, N { 1zu, 1zu }, N { 1zu, 2zu }
            })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, ListEdges) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_edges()),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, NodesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_TRUE(g.has_node(N { 0zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { 0zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { 0zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 0zu, 3zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 1zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { 2zu, 3zu }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, RightEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 0zu, 3zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 1zu, 3zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, DownEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, DiagEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 3zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 3zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, FreeRideEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 2zu }, { 1zu, 2zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetOutputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 0zu, 0zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 1zu, 2zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 0zu, 2zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 1zu, 0zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } },
            })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetInputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 0zu, 0zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 1zu, 2zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 0zu, 2zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 1zu, 0zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }
            })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetOutputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        
        EXPECT_EQ(g.get_out_degree(N { 0zu, 0zu }), 4);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu }), 0);
        EXPECT_EQ(g.get_out_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetInputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_in_degree(N { 0zu, 0zu }), 0);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu }), 5);
        EXPECT_EQ(g.get_in_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, RowWalk) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
    
        using G = std::remove_cvref_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        EXPECT_EQ(
            (copy_to_vector(g.row_nodes(0u))),
            (std::vector<N> {
                N { 0zu, 0zu },
                N { 0zu, 1zu },
                N { 0zu, 2zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(g.row_nodes(1u))),
            (std::vector<N> {
                N { 1zu, 0zu },
                N { 1zu, 1zu },
                N { 1zu, 2zu }
            })
        );

        EXPECT_EQ(g.resident_nodes_capacity, 2zu);
        EXPECT_EQ(g.resident_nodes().size(), 2zu);
        EXPECT_EQ(g.resident_nodes()[0], (N { 0zu, 0zu }));
        EXPECT_EQ(g.resident_nodes()[1], (N { 1zu, 2zu }));

        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 0zu, 1zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 0zu, 2zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );


        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 0zu, 1zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 0zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> { })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, CreateViaFactory) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g1 {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        auto g2 {
            create_pairwise_overlap_alignment_graph<is_debug_mode(), std::size_t>(
                seq1,
                seq2,
                substitution_scorer,
                gap_scorer,
                freeride_scorer
            )
        };
        EXPECT_TRUE((std::is_same_v<decltype(g1), decltype(g2)>));
    }
}