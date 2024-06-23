#include <cstddef>
#include <algorithm>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;

    auto match_lookup {
        [](
            const auto& edge,
            const char& down_elem,
            const char& right_elem
        ) -> std::float64_t {
            if (down_elem == right_elem) {
                return 1.0f64;
            } else {
                return -1.0f64;
            }
        }
    };
    auto indel_lookup {
        [](
            const auto& edge
        ) -> std::float64_t {
            return 0.0f64;
        }
    };
    auto freeride_lookup {
        [](
            const auto& edge
        ) -> std::float64_t {
            return 0.0f64;
        }
    };
    
    struct graph_bundle {
        std::string down_seq;
        std::string right_seq;
        pairwise_local_alignment_graph<std::string, std::string> backing_g;
        reversed_sliceable_pairwise_alignment_graph<decltype(backing_g)> reversed_g;

        graph_bundle(
            std::string _down_seq,
            std::string _right_seq
        )
        : down_seq{_down_seq}
        , right_seq{_right_seq}
        , backing_g{
            down_seq,
            right_seq,
            match_lookup,
            indel_lookup,
            freeride_lookup
        }
        , reversed_g{backing_g} {}
    };

    TEST(ReversedPairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle{"a", "ac"}.reversed_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph<G>);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                std::pair{0zu, 0zu}, std::pair{0zu, 1zu}, std::pair{0zu, 2zu},
                std::pair{1zu, 0zu}, std::pair{1zu, 1zu}, std::pair{1zu, 2zu}
            })
        );
    }

    TEST(ReversedPairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        using E = typename std::remove_reference_t<decltype(g)>::E;

        auto e = g.get_edges();
        std::multiset<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
        for (auto _e : e) {
            actual.insert(_e);
        }
        EXPECT_EQ(
            actual,
            (std::multiset<E> {
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 2zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 2zu} } },

                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 1zu}, std::pair{0zu, 2zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{1zu, 0zu}, std::pair{1zu, 1zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 1zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 2zu} } },

                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{1zu, 0zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} } }
            })
        );
    }

    TEST(ReversedPairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };

        EXPECT_TRUE(g.has_node({0zu, 0zu}));
        EXPECT_TRUE(g.has_node({0zu, 1zu}));
        EXPECT_TRUE(g.has_node({0zu, 2zu}));
        EXPECT_FALSE(g.has_node({0zu, 3zu}));
        EXPECT_TRUE(g.has_node({1zu, 0zu}));
        EXPECT_TRUE(g.has_node({1zu, 1zu}));
        EXPECT_TRUE(g.has_node({1zu, 2zu}));
        EXPECT_FALSE(g.has_node({1zu, 3zu}));
        EXPECT_FALSE(g.has_node({2zu, 3zu}));
    }

    TEST(ReversedPairwiseAlignmentGraphTest, RightEdgesExist) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {0zu, 1zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 1zu}, {0zu, 2zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 2zu}, {0zu, 3zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{1zu, 0zu}, {1zu, 1zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{1zu, 1zu}, {1zu, 2zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 2zu}, {1zu, 3zu}}}));
    }

    TEST(ReversedPairwiseAlignmentGraphTest, DownEdgesExist) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {1zu, 0zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 0zu}, {2zu, 0zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 1zu}, {1zu, 1zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 1zu}, {2zu, 1zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 2zu}, {1zu, 2zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 2zu}, {2zu, 2zu}}}));
    }

    TEST(ReversedPairwiseAlignmentGraphTest, DiagEdgesExist) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {1zu, 1zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 0zu}, {2zu, 1zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0zu, 1zu}, {1zu, 2zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 1zu}, {2zu, 2zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 2zu}, {1zu, 3zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 2zu}, {2zu, 3zu}}}));
    }

    TEST(ReversedPairwiseAlignmentGraphTest, FreeRideEdgesExist) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };

        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 0zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 2zu} } }));

        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1zu, 0zu}, std::pair{1zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} } }));
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1zu, 2zu}, std::pair{1zu, 2zu} } }));
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 1zu, 2zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            std::vector<E> expected {
                { edge_type::FREE_RIDE, { {0zu, 0zu}, {1zu, 2zu} } },
                { edge_type::FREE_RIDE, { {0zu, 1zu}, {1zu, 2zu} } },
                { edge_type::FREE_RIDE, { {0zu, 2zu}, {1zu, 2zu} } },
                { edge_type::FREE_RIDE, { {1zu, 0zu}, {1zu, 2zu} } },
                { edge_type::FREE_RIDE, { {1zu, 1zu}, {1zu, 2zu} } },
                { edge_type::NORMAL, { {0zu, 1zu}, {1zu, 2zu} } },
                { edge_type::NORMAL, { {0zu, 2zu}, {1zu, 2zu} } },
                { edge_type::NORMAL, { {1zu, 1zu}, {1zu, 2zu} } }
            };
            EXPECT_EQ(actual, expected);
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 0zu, 0zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 1zu, 1zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::FREE_RIDE, {{0zu, 0zu}, {1zu, 1zu} } },
                    { edge_type::NORMAL, { {0zu, 0zu}, {1zu, 1zu} } },
                    { edge_type::NORMAL, { {0zu, 1zu}, {1zu, 1zu} } },
                    { edge_type::NORMAL, { {1zu, 0zu}, {1zu, 1zu} } }
                })
            );
        }
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( std::pair{ 1zu, 2zu } )) {
                actual.push_back(_e);
            }
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( { 0zu, 0zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::FREE_RIDE, {{0zu, 0zu}, {0zu, 1zu} } },
                    { edge_type::FREE_RIDE, {{0zu, 0zu}, {0zu, 2zu} } },
                    { edge_type::FREE_RIDE, {{0zu, 0zu}, {1zu, 0zu} } },
                    { edge_type::FREE_RIDE, {{0zu, 0zu}, {1zu, 1zu} } },
                    { edge_type::FREE_RIDE, {{0zu, 0zu}, {1zu, 2zu} } },
                    { edge_type::NORMAL, {{0zu, 0zu}, {0zu, 1zu} } },
                    { edge_type::NORMAL, {{0zu, 0zu}, {1zu, 0zu} } },
                    { edge_type::NORMAL, {{0zu, 0zu}, {1zu, 1zu} } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( std::pair{0zu, 1zu} )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::FREE_RIDE, { {0zu, 1zu}, {1zu, 2zu} } },
                    { edge_type::NORMAL, { {0zu, 1zu}, {0zu, 2zu} } },
                    { edge_type::NORMAL, { {0zu, 1zu}, {1zu, 1zu} } },
                    { edge_type::NORMAL, { {0zu, 1zu}, {1zu, 2zu} } }
                })
            );
        }
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
        
        EXPECT_EQ(g.get_out_degree(std::pair{ 0zu, 0zu } ), 0);
        EXPECT_EQ(g.get_out_degree(std::pair{ 1zu, 2zu } ), 8);
        EXPECT_EQ(g.get_out_degree(std::pair{ 0zu, 2zu } ), 2);
        EXPECT_EQ(g.get_out_degree(std::pair{ 1zu, 0zu } ), 2);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };

        EXPECT_EQ(g.get_in_degree(std::pair{ 0zu, 0zu } ), 8);
        EXPECT_EQ(g.get_in_degree(std::pair{ 1zu, 2zu } ), 0);
        EXPECT_EQ(g.get_in_degree(std::pair{ 0zu, 2zu } ), 2);
        EXPECT_EQ(g.get_in_degree(std::pair{ 1zu, 0zu } ), 2);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, SlicedWalk) {
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

        graph_bundle g_bundle{"a", "ac"};
        auto g { g_bundle.reversed_g };
    
        using G = std::decay_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        EXPECT_EQ(G::slice_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 3zu);
        EXPECT_EQ(g.slice_first_node(0zu), (N { 1zu, 2zu }));
        EXPECT_EQ(g.slice_first_node(0zu, 0zu), (N { 1zu, 2zu }));
        EXPECT_EQ(g.slice_first_node(0zu, 1zu), (N { 1zu, 1zu }));
        EXPECT_EQ(g.slice_first_node(0zu, 2zu), (N { 1zu, 0zu }));
        EXPECT_EQ(g.slice_last_node(0zu), (N { 1zu, 0zu }));
        EXPECT_EQ(g.slice_last_node(0zu, 2zu), (N { 1zu, 0zu }));
        EXPECT_EQ(g.slice_last_node(0zu, 1zu), (N { 1zu, 1zu }));
        EXPECT_EQ(g.slice_last_node(0zu, 0zu), (N { 1zu, 2zu }));
        EXPECT_EQ(g.slice_first_node(1zu), (N { 0zu, 2zu }));
        EXPECT_EQ(g.slice_last_node(1zu), (N { 0zu, 0zu }));

        EXPECT_EQ(G::resident_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 2zu);
        EXPECT_EQ(g.resident_nodes().size(), 2zu);
        EXPECT_EQ(g.resident_nodes()[0], (N { 0zu, 0zu }));
        EXPECT_EQ(g.resident_nodes()[1], (N { 1zu, 2zu }));

        EXPECT_EQ(g.slice_prev_node(N { 0zu, 0zu }), (N { 0zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { 0zu, 1zu }), (N { 0zu, 2zu }));
        EXPECT_EQ(g.slice_prev_node(N { 1zu, 0zu }), (N { 1zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { 1zu, 1zu }), (N { 1zu, 2zu }));

        EXPECT_EQ(g.slice_next_node(N { 0zu, 1zu }), (N { 0zu, 0zu }));
        EXPECT_EQ(g.slice_next_node(N { 0zu, 2zu }), (N { 0zu, 1zu }));
        EXPECT_EQ(g.slice_next_node(N { 1zu, 1zu }), (N { 1zu, 0zu }));
        EXPECT_EQ(g.slice_next_node(N { 1zu, 2zu }), (N { 1zu, 1zu }));

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