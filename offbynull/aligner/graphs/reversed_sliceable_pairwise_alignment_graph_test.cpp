#include <cstddef>
#include <algorithm>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
    auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride() };
    
    struct graph_bundle {
        std::string down_seq;
        std::string right_seq;
        pairwise_local_alignment_graph<true, std::string, std::string> backing_g;
        reversed_sliceable_pairwise_alignment_graph<true, decltype(backing_g)> reversed_g;

        graph_bundle(
            std::string _down_seq,
            std::string _right_seq
        )
        : down_seq { _down_seq }
        , right_seq { _right_seq }
        , backing_g {
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        }
        , reversed_g { backing_g } {}
    };

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle { "a", "ac" }.reversed_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set<N> {
                N { 0zu, 0zu }, N { 0zu, 1zu }, N { 0zu, 2zu },
                N { 1zu, 0zu }, N { 1zu, 1zu }, N { 1zu, 2zu }
            })
        );
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        auto e = g.get_edges();
        std::multiset<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
        for (auto _e : e) {
            actual.insert(_e);
        }
        EXPECT_EQ(
            actual,
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
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },

                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

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

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, RightEdgesExist) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 0zu, 3zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 1zu, 3zu } } }));
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, DownEdgesExist) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } }));
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, DiagEdgesExist) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 3zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 3zu } } }));
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, FreeRideEdgesExist) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));

        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 2zu }, { 1zu, 2zu } } }));
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs(N { 1zu, 2zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            std::vector<E> expected {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            };
            EXPECT_EQ(actual, expected);
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs(N { 0zu, 0zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs(N { 1zu, 1zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } },
                    E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } },
                    E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } },
                    E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } }
                })
            );
        }
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 1zu, 2zu })) {
                actual.push_back(_e);
            }
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 0zu, 0zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
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
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 0zu, 1zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } },
                    E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } },
                    E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } },
                    E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }
                })
            );
        }
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;
        
        EXPECT_EQ(g.get_out_degree(N { 0zu, 0zu }), 0);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu }), 8);
        EXPECT_EQ(g.get_out_degree(N { 0zu, 2zu }), 2);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(g.get_in_degree(N { 0zu, 0zu }), 8);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu }), 0);
        EXPECT_EQ(g.get_in_degree(N { 0zu, 2zu }), 2);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGReversedSliceablePairwiseAlignmentGraphTest, SlicedWalk) {
        auto to_vector {
            [](auto &&r) {
                auto it { r.begin() };
                std::vector<std::decay_t<decltype(*it)>> ret {};
                while (it != r.end()) {
                    ret.push_back(*it);
                    ++it;
                }
                return ret;
            }
        };

        graph_bundle g_bundle { "a", "ac" };
        auto g { g_bundle.reversed_g };
    
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            (to_vector(g.slice_nodes(0u))),
            (std::vector<N> {
                N { 1zu, 2zu },
                N { 1zu, 1zu },
                N { 1zu, 0zu }
            })
        );
        EXPECT_EQ(
            (to_vector(g.slice_nodes(1u))),
            (std::vector<N> {
                N { 0zu, 2zu },
                N { 0zu, 1zu },
                N { 0zu, 0zu }
            })
        );

        EXPECT_EQ(g.resident_nodes_capacity, 2zu);
        EXPECT_EQ(g.resident_nodes().size(), 2zu);
        EXPECT_EQ(g.resident_nodes()[0], (N { 0zu, 0zu }));
        EXPECT_EQ(g.resident_nodes()[1], (N { 1zu, 2zu }));

        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 1zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 2zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );


        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 1zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 2zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } },
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 2zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }
            })
        );
    }
}