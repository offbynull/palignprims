#include <cstddef>
#include <stdfloat>
#include <string>
#include <set>
#include <type_traits>
#include <vector>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::create_grid_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_set;
    using offbynull::utils::copy_to_vector;
    using offbynull::utils::is_debug_mode;

    auto substitution_scorer {
        simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64)
    };
    auto gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(0.0f64) };

    TEST(OAGGridGraphTest, ConceptCheck) {
        using G = grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            std::string,
            std::string,
            decltype(substitution_scorer),
            decltype(gap_scorer)
        >;
        static_assert(offbynull::aligner::graph::graph::graph<G>);
    }

    TEST(OAGGridGraphTest, ListNodes) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                N { 0zu, 0zu }, N { 0zu, 1zu }, N { 0zu, 2zu },
                N { 1zu, 0zu }, N { 1zu, 1zu }, N { 1zu, 2zu }
            })
        );
    }

    TEST(OAGGridGraphTest, ListEdges) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_edges()),
            (std::set<E> {
                E { N { 0zu, 0zu }, N { 0zu, 1zu } },
                E { N { 0zu, 1zu }, N { 0zu, 2zu } },
                E { N { 1zu, 0zu }, N { 1zu, 1zu } },
                E { N { 1zu, 1zu }, N { 1zu, 2zu } },
                E { N { 0zu, 0zu }, N { 1zu, 0zu } },
                E { N { 0zu, 1zu }, N { 1zu, 1zu } },
                E { N { 0zu, 2zu }, N { 1zu, 2zu } },
                E { N { 0zu, 0zu }, N { 1zu, 1zu } },
                E { N { 0zu, 1zu }, N { 1zu, 2zu } }
            })
        );
    }

    TEST(OAGGridGraphTest, NodesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_node({ 0zu, 0zu }));
        EXPECT_TRUE(g.has_node({ 0zu, 1zu }));
        EXPECT_TRUE(g.has_node({ 0zu, 2zu }));
        EXPECT_FALSE(g.has_node({ 0zu, 3zu }));
        EXPECT_TRUE(g.has_node({ 1zu, 0zu }));
        EXPECT_TRUE(g.has_node({ 1zu, 1zu }));
        EXPECT_TRUE(g.has_node({ 1zu, 2zu }));
        EXPECT_FALSE(g.has_node({ 1zu, 3zu }));
        EXPECT_FALSE(g.has_node({ 2zu, 3zu }));
    }

    TEST(OAGGridGraphTest, RightEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({ { 0zu, 0zu }, { 0zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 1zu }, { 0zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 0zu, 2zu }, { 0zu, 3zu } }));
        EXPECT_TRUE(g.has_edge({ { 1zu, 0zu }, { 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 1zu, 1zu }, { 1zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 2zu }, { 1zu, 3zu } }));
    }

    TEST(OAGGridGraphTest, DownEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({ { 0zu, 0zu }, { 1zu, 0zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 0zu }, { 2zu, 0zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 1zu }, { 1zu, 1zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 1zu }, { 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 2zu }, { 1zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 2zu }, { 2zu, 2zu } }));
    }

    TEST(OAGGridGraphTest, DiagEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({ { 0zu, 0zu }, { 1zu, 1zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 0zu }, { 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 1zu }, { 1zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 1zu }, { 2zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 0zu, 2zu }, { 1zu, 3zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 2zu }, { 2zu, 3zu } }));
    }

    TEST(OAGGridGraphTest, GetOutputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 0zu, 0zu })),
            (std::set<E> {
                E { N { 0zu, 0zu }, N { 0zu, 1zu } },
                E { N { 0zu, 0zu }, N { 1zu, 0zu } },
                E { N { 0zu, 0zu }, N { 1zu, 1zu } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 1zu, 2zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 0zu, 2zu })),
            (std::set<E> {
                E { N { 0zu, 2zu }, N { 1zu, 2zu } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 1zu, 0zu })),
            (std::set<E> {
                E { N { 1zu, 0zu }, N { 1zu, 1zu } }
            })
        );
    }

    TEST(OAGGridGraphTest, GetInputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
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
                E { N { 0zu, 2zu }, N { 1zu, 2zu } },
                E { N { 1zu, 1zu }, N { 1zu, 2zu } },
                E { N { 0zu, 1zu }, N { 1zu, 2zu } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 0zu, 2zu })),
            (std::set<E> {
                E { N { 0zu, 1zu }, N { 0zu, 2zu } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 1zu, 0zu })),
            (std::set<E> {
                E { N { 0zu, 0zu }, N { 1zu, 0zu } }
            })
        );
    }

    TEST(OAGGridGraphTest, GetOutputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_out_degree(N { 0zu, 0zu }), 3);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu }), 0);
        EXPECT_EQ(g.get_out_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 0zu }), 1);
    }

    TEST(OAGGridGraphTest, GetInputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_in_degree(N { 0zu, 0zu }), 0);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu }), 3);
        EXPECT_EQ(g.get_in_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 0zu }), 1);
    }

    TEST(OAGGridGraphTest, RowWalk) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

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

        EXPECT_EQ(g.resident_nodes_capacity, 0zu);
        EXPECT_EQ(g.resident_nodes().size(), 0zu);

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
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 0zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );
    }

    TEST(OAGGridGraphTest, CreateViaFactory) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g1 {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        auto g2 { create_grid_graph<is_debug_mode(), std::size_t>(seq1, seq2, substitution_scorer, gap_scorer) };
        EXPECT_TRUE((std::is_same_v<decltype(g1), decltype(g2)>));
    }
}