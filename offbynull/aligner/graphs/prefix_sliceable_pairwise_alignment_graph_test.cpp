#include <cstddef>
#include <ranges>
#include <string>
#include <set>
#include <vector>
#include <stdfloat>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_vector;
    using offbynull::utils::copy_to_set;
    using offbynull::utils::copy_to_multiset;

    auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
    auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride() };
    
    struct graph_bundle {
        std::string down_seq;
        std::string right_seq;
        pairwise_local_alignment_graph<
            true,
            std::size_t,
            std::float64_t,
            std::string,
            std::string,
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > backing_g;
        prefix_sliceable_pairwise_alignment_graph<true, decltype(backing_g)> prefix_g;

        graph_bundle(
            std::string down_seq_,
            std::string right_seq_,
            decltype(backing_g)::N new_leaf_node
        )
        : down_seq { down_seq_ }
        , right_seq { right_seq_ }
        , backing_g {
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        }
        , prefix_g { backing_g, new_leaf_node } {}
    };

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle { "234567", "2345678", { 1zu, 2zu } }.prefix_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using N = typename decltype(g)::N;

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set<N> {
                N { 0zu, 0zu }, N { 0zu, 1zu }, N { 0zu, 2zu },
                N { 1zu, 0zu }, N { 1zu, 1zu }, N { 1zu, 2zu }
            })
        );
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_multiset(g.get_edges()),
            (std::multiset<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } },

                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }
            })
        );
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

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

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, RightEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 0zu, 3zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 1zu, 3zu } } }));
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, DownEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } }));
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, DiagEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 3zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 3zu } } }));
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, FreeRideEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));

        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } }));  // culled from backing graph
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } }));  // culled from backing graph
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } }));  // culled from backing graph
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } }));  // culled from backing graph
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 2zu }, { 1zu, 2zu } } }));
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 0zu, 0zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } },
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
            copy_to_set(g.get_outputs(N { 0zu, 1zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }
            })
        );
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 0zu, 0zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 1zu, 2zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 0zu, 2zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }
            })
        );
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_out_degree(N { 0zu, 0zu }), 8);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu }), 0);
        EXPECT_EQ(g.get_out_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 0zu }), 1);
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_in_degree(N { 0zu, 0zu }), 0);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu }), 4);
        EXPECT_EQ(g.get_in_degree(N { 0zu, 2zu }), 2);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGPrefixSliceablePairwiseAlignmentGraphTest, SlicedWalk) {
        graph_bundle g_bundle { "234567", "2345678", { 1zu, 2zu } };
        auto g { g_bundle.prefix_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            (copy_to_vector(g.slice_nodes(0u))),
            (std::vector<N> {
                N { 0zu, 0zu },
                N { 0zu, 1zu },
                N { 0zu, 2zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(g.slice_nodes(1u))),
            (std::vector<N> {
                N { 1zu, 0zu },
                N { 1zu, 1zu },
                N { 1zu, 2zu }
            })
        );

        EXPECT_EQ(g.resident_nodes_capacity, 2zu); // directly proxied from backing graph
        auto resident_nodes { g.resident_nodes() };
        EXPECT_EQ(std::ranges::distance(resident_nodes.begin(), resident_nodes.end()), 1zu);
        auto resident_nodes_it { resident_nodes.begin() };
        EXPECT_EQ(*resident_nodes_it, (N { 0zu, 0zu }));
        ++resident_nodes_it;
        EXPECT_EQ(resident_nodes_it, resident_nodes.end());

        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 0zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 0zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
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
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 0zu, 2zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } },
            })
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
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }
            })
        );
    };
}