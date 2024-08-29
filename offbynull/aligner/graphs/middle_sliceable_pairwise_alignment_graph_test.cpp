#include <cstddef>
#include <stdfloat>
#include <string>
#include <set>
#include <vector>
#include <ranges>
#include <type_traits>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::create_middle_sliceable_pairwise_alignment_graph;
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
        middle_sliceable_pairwise_alignment_graph<true, decltype(backing_g)> middle_g;

        graph_bundle(
            std::string down_seq_,
            std::string right_seq_,
            decltype(backing_g)::N root_node,
            decltype(backing_g)::N leaf_node
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
        , middle_g { backing_g, root_node, leaf_node } {}
    };

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } }.middle_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                N { 1zu, 1zu }, N { 1zu, 2zu },
                N { 2zu, 1zu }, N { 2zu, 2zu },
            })
        );
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_multiset(g.get_edges()),
            (std::multiset<E> {
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } },
                E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } },
                E { edge_type::NORMAL, { { 2zu, 1zu }, { 2zu, 2zu } } },
            })
        );
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;

        EXPECT_FALSE(g.has_node(N { 0zu, 0zu }));
        EXPECT_FALSE(g.has_node(N { 0zu, 1zu }));
        EXPECT_FALSE(g.has_node(N { 0zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 0zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { 1zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 1zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { 2zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { 2zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { 2zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 2zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { 3zu, 0zu }));
        EXPECT_FALSE(g.has_node(N { 3zu, 1zu }));
        EXPECT_FALSE(g.has_node(N { 3zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 3zu, 3zu }));
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, EdgesExist) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 2zu, 1zu }, { 2zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 7zu }, { 6zu, 8zu } } }));
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 1zu, 1zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 2zu, 2zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 1zu, 2zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } }
            })
        );
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 1zu, 1zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 2zu, 2zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } },
                E { edge_type::NORMAL, { { 2zu, 1zu }, { 2zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 1zu, 2zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_out_degree(N { 1zu, 1zu }), 3);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu }), 1);
        EXPECT_EQ(g.get_out_degree(N { 2zu, 1zu }), 1);
        EXPECT_EQ(g.get_out_degree(N { 2zu, 2zu }), 0);
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_in_degree(N { 1zu, 1zu }), 0);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu }), 1);
        EXPECT_EQ(g.get_in_degree(N { 2zu, 1zu }), 1);
        EXPECT_EQ(g.get_in_degree(N { 2zu, 2zu }), 3);
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, SlicedWalk) {
        graph_bundle g_bundle { "abc", "acc", { 1zu, 1zu }, { 2zu, 2zu } };
        auto g { g_bundle.middle_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            (copy_to_vector(g.slice_nodes(0u))),
            (std::vector<N> {
                N { 1zu, 1zu },
                N { 1zu, 2zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(g.slice_nodes(1u))),
            (std::vector<N> {
                N { 2zu, 1zu },
                N { 2zu, 2zu }
            })
        );

        EXPECT_EQ(g.resident_nodes_capacity, 2zu); // directly proxied from backing graph
        auto resident_nodes { g.resident_nodes() };
        EXPECT_EQ(std::ranges::distance(resident_nodes.begin(), resident_nodes.end()), 0zu);
        auto resident_nodes_it { resident_nodes.begin() };
        EXPECT_EQ(resident_nodes_it, resident_nodes.end());

        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 2zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 2zu, 2zu })),
            (std::vector<E> {})
        );

        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 2zu, 2zu })),
            (std::vector<E> { })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 2zu, 1zu })),
            (std::vector<E> { })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> { })
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> { })
        );
    }

    TEST(OAGMiddleSliceablePairwiseAlignmentGraphTest, CreateViaFactory) {
        std::string seq1 { "234567" };
        std::string seq2 { "2345678" };

        graph_bundle g_bundle { seq1, seq2, { 1zu, 1zu }, { 2zu, 2zu } };
        auto g1 { g_bundle.middle_g };
        auto g2 {
            create_middle_sliceable_pairwise_alignment_graph<true>(
                g_bundle.backing_g,
                { 1zu, 1zu },
                { 2zu, 2zu }
            )
        };

        EXPECT_TRUE((std::is_same_v<decltype(g1), decltype(g2)>));
    }
}