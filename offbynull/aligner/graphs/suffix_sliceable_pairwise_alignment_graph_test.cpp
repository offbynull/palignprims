#include <cstddef>
#include <algorithm>
#include <stdfloat>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto substitution_scorer { simple_scorer<char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<char, char, std::float64_t>::create_gap(0.0f64) };
    auto freeride_scorer { simple_scorer<char, char, std::float64_t>::create_freeride() };

    struct graph_bundle {
        std::string down_seq;
        std::string right_seq;
        pairwise_local_alignment_graph<std::string, std::string> backing_g;
        suffix_sliceable_pairwise_alignment_graph<decltype(backing_g)> suffix_g;

        graph_bundle(
            std::string _down_seq,
            std::string _right_seq,
            decltype(backing_g)::N new_root_node
        )
        : down_seq{_down_seq}
        , right_seq{_right_seq}
        , backing_g{
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        }
        , suffix_g{backing_g, new_root_node} {}
    };

    TEST(SuffixPairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle { "234567", "2345678", { 5zu, 5zu }  }.suffix_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(SuffixPairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                std::pair{5zu, 5zu}, std::pair{5zu, 6zu}, std::pair{5zu, 7zu},
                std::pair{6zu, 5zu}, std::pair{6zu, 6zu}, std::pair{6zu, 7zu}
            })
        );
    }

    TEST(SuffixPairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        auto e = g.get_edges();
        std::multiset<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
        for (auto _e : e) {
            actual.insert(_e);
        }
        EXPECT_EQ(
            actual,
            (std::multiset<E> {
                // culled from backing graph
                // edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} } },
                // edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 2zu} } },
                // edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} } },
                // edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} } },
                // edge { edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 2zu} } },

                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 5zu}, std::pair{5zu, 6zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 6zu}, std::pair{5zu, 7zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{6zu, 5zu}, std::pair{6zu, 6zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{6zu, 6zu}, std::pair{6zu, 7zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 5zu}, std::pair{6zu, 5zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 6zu}, std::pair{6zu, 6zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 7zu}, std::pair{6zu, 7zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 5zu}, std::pair{6zu, 6zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{5zu, 6zu}, std::pair{6zu, 7zu} } },

                edge { edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{6zu, 7zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{5zu, 6zu}, std::pair{6zu, 7zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{5zu, 7zu}, std::pair{6zu, 7zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{6zu, 5zu}, std::pair{6zu, 7zu} } },
                edge { edge_type::FREE_RIDE, std::pair { std::pair{6zu, 6zu}, std::pair{6zu, 7zu} } }
            })
        );
    }

    TEST(SuffixPairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_FALSE(g.has_node({0zu, 0zu}));
        EXPECT_FALSE(g.has_node({0zu, 1zu}));

        EXPECT_TRUE(g.has_node({5zu, 5zu}));
        EXPECT_TRUE(g.has_node({5zu, 6zu}));
        EXPECT_TRUE(g.has_node({5zu, 7zu}));
        EXPECT_FALSE(g.has_node({5zu, 8zu}));
        EXPECT_TRUE(g.has_node({6zu, 5zu}));
        EXPECT_TRUE(g.has_node({6zu, 6zu}));
        EXPECT_TRUE(g.has_node({6zu, 7zu}));
        EXPECT_FALSE(g.has_node({6zu, 8zu}));
        EXPECT_FALSE(g.has_node({7zu, 8zu}));
    }

    TEST(SuffixPairwiseAlignmentGraphTest, RightEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {0zu, 1zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 1zu}, {0zu, 2zu}}}));

        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 5zu}, {5zu, 6zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 6zu}, {5zu, 7zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{5zu, 7zu}, {5zu, 8zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{6zu, 5zu}, {6zu, 6zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{6zu, 6zu}, {6zu, 7zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 7zu}, {6zu, 8zu}}}));
    }

    TEST(SuffixPairwiseAlignmentGraphTest, DownEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {1zu, 0zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 1zu}, {1zu, 1zu}}}));

        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 5zu}, {6zu, 5zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 5zu}, {7zu, 5zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 6zu}, {6zu, 6zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 6zu}, {7zu, 6zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 7zu}, {6zu, 7zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 7zu}, {7zu, 7zu}}}));
    }

    TEST(SuffixPairwiseAlignmentGraphTest, DiagEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {1zu, 1zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1zu, 0zu}, {2zu, 1zu}}}));

        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 5zu}, {6zu, 6zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 5zu}, {7zu, 6zu}}}));
        EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{5zu, 6zu}, {6zu, 7zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 6zu}, {7zu, 7zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{5zu, 7zu}, {6zu, 8zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 7zu}, {7zu, 38}}}));
    }

    TEST(SuffixPairwiseAlignmentGraphTest, FreeRideEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{5zu, 5zu} } }));
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{5zu, 6zu} } }));
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{5zu, 7zu} } }));
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{6zu, 5zu} } }));
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{6zu, 6zu} } }));

        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0zu, 0zu}, std::pair{6zu, 7zu} } }));

        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 5zu}, std::pair{6zu, 7zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 6zu}, std::pair{6zu, 7zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{5zu, 7zu}, std::pair{6zu, 7zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{6zu, 5zu}, std::pair{6zu, 7zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{6zu, 6zu}, std::pair{6zu, 7zu} } }));
        EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{6zu, 7zu}, std::pair{6zu, 7zu} } }));
    }

    TEST(SuffixPairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 5zu, 5zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            std::vector<E> expected {
                { edge_type::FREE_RIDE, {{5zu, 5zu}, {6zu, 7zu} } },
                { edge_type::NORMAL, {{5zu, 5zu}, {5zu, 6zu} } },
                { edge_type::NORMAL, {{5zu, 5zu}, {6zu, 5zu} } },
                { edge_type::NORMAL, {{5zu, 5zu}, {6zu, 6zu} } }
            };
            EXPECT_EQ(actual, expected);
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 6zu, 7zu } )) {
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
            for (auto _e : g.get_outputs( { 5zu, 6zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::FREE_RIDE, {{5zu, 6zu}, {6zu, 7zu} } },
                    { edge_type::NORMAL, { {5zu, 6zu}, {5zu, 7zu} } },
                    { edge_type::NORMAL, { {5zu, 6zu}, {6zu, 6zu} } },
                    { edge_type::NORMAL, { {5zu, 6zu}, {6zu, 7zu} } }
                })
            );
        }
    }

    TEST(SuffixPairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( std::pair{ 5zu, 5zu } )) {
                actual.push_back(_e);
            }
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( { 6zu, 7zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::FREE_RIDE, {{5zu, 5zu}, {6zu, 7zu} } },
                    { edge_type::FREE_RIDE, {{5zu, 6zu}, {6zu, 7zu} } },
                    { edge_type::FREE_RIDE, {{5zu, 7zu}, {6zu, 7zu} } },
                    { edge_type::FREE_RIDE, {{6zu, 5zu}, {6zu, 7zu} } },
                    { edge_type::FREE_RIDE, {{6zu, 6zu}, {6zu, 7zu} } },
                    { edge_type::NORMAL, {{5zu, 6zu}, {6zu, 7zu} } },
                    { edge_type::NORMAL, {{5zu, 7zu}, {6zu, 7zu} } },
                    { edge_type::NORMAL, {{6zu, 6zu}, {6zu, 7zu} } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( std::pair{5zu, 7zu} )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::NORMAL, { {5zu, 6zu}, {5zu, 7zu} } }
                })
            );
        }
    }

    TEST(SuffixPairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_EQ(g.get_out_degree(std::pair{ 5zu, 5zu } ), 4);
        EXPECT_EQ(g.get_out_degree(std::pair{ 6zu, 7zu } ), 0);
        EXPECT_EQ(g.get_out_degree(std::pair{ 5zu, 7zu } ), 2);
        EXPECT_EQ(g.get_out_degree(std::pair{ 6zu, 5zu } ), 2);
    }

    TEST(SuffixPairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        EXPECT_EQ(g.get_in_degree(std::pair{ 5zu, 5zu } ), 0);
        EXPECT_EQ(g.get_in_degree(std::pair{ 6zu, 7zu } ), 8);
        EXPECT_EQ(g.get_in_degree(std::pair{ 5zu, 7zu } ), 1);
        EXPECT_EQ(g.get_in_degree(std::pair{ 6zu, 5zu } ), 1);
    }

    TEST(SuffixPairwiseAlignmentGraphTest, SlicedWalk) {
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

        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using G = std::decay_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        EXPECT_EQ(G::limits(g.grid_down_cnt, g.grid_right_cnt).max_slice_nodes_cnt, 3zu);
        EXPECT_EQ(g.slice_first_node(0zu), (N { 5zu, 5zu }));
        EXPECT_EQ(g.slice_last_node(0zu), (N { 5zu, 7zu }));
        EXPECT_EQ(g.slice_first_node(1zu), (N { 6zu, 5zu }));
        EXPECT_EQ(g.slice_last_node(1zu), (N {  6zu, 7zu }));

        EXPECT_EQ(G::limits(g.grid_down_cnt, g.grid_right_cnt).max_resident_nodes_cnt, 2zu); // directly proxied from backing graph
        auto resident_nodes { g.resident_nodes() };
        EXPECT_EQ(std::distance(resident_nodes.begin(), resident_nodes.end()), 1zu);
        auto resident_nodes_it { resident_nodes.begin() };
        EXPECT_EQ(*resident_nodes_it, (N { 6zu, 7zu }));
        ++resident_nodes_it;
        EXPECT_EQ(resident_nodes_it, resident_nodes.end());

        EXPECT_EQ(g.slice_next_node(N { 5zu, 5zu }), (N { 5zu, 6zu }));
        EXPECT_EQ(g.slice_next_node(N { 5zu, 6zu }), (N { 5zu, 7zu }));
        EXPECT_EQ(g.slice_next_node(N { 6zu, 5zu }), (N { 6zu, 6zu }));
        EXPECT_EQ(g.slice_next_node(N { 6zu, 6zu }), (N { 6zu, 7zu }));

        EXPECT_EQ(g.slice_prev_node(N { 5zu, 6zu }), (N { 5zu, 5zu }));
        EXPECT_EQ(g.slice_prev_node(N { 5zu, 7zu }), (N { 5zu, 6zu }));
        EXPECT_EQ(g.slice_prev_node(N { 6zu, 6zu }), (N { 6zu, 5zu }));
        EXPECT_EQ(g.slice_prev_node(N { 6zu, 7zu }), (N { 6zu, 6zu }));

        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 5zu, 5zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 5zu, 6zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 5zu, 6zu }, { 6zu, 7zu } } },
                 E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 5zu, 7zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 5zu, 7zu }, { 6zu, 7zu } } },
                 E { edge_type::NORMAL, { { 5zu, 7zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 6zu, 5zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 6zu, 5zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 6zu, 6zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 6zu, 6zu }, { 6zu, 7zu } } },
                 E { edge_type::NORMAL, { { 6zu, 6zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 6zu, 7zu })),
            (std::vector<E> {})
        );


        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 5zu, 5zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 5zu, 6zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 5zu, 7zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 6zu, 5zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 6zu, 6zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 6zu, 7zu })),
            (std::vector<E> {})
        );
    }

    TEST(SuffixPairwiseAlignmentGraphTest, PrintBasic) {
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

        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using G = std::decay_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        std::cout << "res" << std::endl;
        for (auto&& n : g.resident_nodes()) {
            std::cout << std::get<0>(n) << ',' << std::get<1>(n) << " ";
        }
        std::cout << std::endl;

        std::cout << "norm0" << std::endl;
        for (auto&& n : g.slice_nodes(0zu)) {
            std::cout << std::get<0>(n) << ',' << std::get<1>(n) << " ";
        }
        std::cout << std::endl;

        std::cout << "norm1" << std::endl;
        for (auto&& n : g.slice_nodes(1zu)) {
            std::cout << std::get<0>(n) << ',' << std::get<1>(n) << " ";
        }
        std::cout << std::endl;

        std::cout << "root" << std::endl;
        std::cout << std::get<0>(g.get_root_node()) << ',' << std::get<1>(g.get_root_node()) << " ";
        std::cout << std::endl;

        std::cout << "leaf" << std::endl;
        std::cout << std::get<0>(g.get_leaf_node()) << ',' << std::get<1>(g.get_leaf_node()) << " ";
        std::cout << std::endl;
    }
}