#include <cstddef>
#include <algorithm>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;

    template <typename BACKING_G>
    struct created_graph {
        BACKING_G backing_g;
        reversed_sliceable_pairwise_alignment_graph<BACKING_G> reversed_g;

        created_graph(BACKING_G _backing_g)
        : backing_g {_backing_g}
        , reversed_g {backing_g} {}
    };

    template<typename ED, typename INDEX = unsigned int, bool error_check = true>
    auto create_vector(INDEX down_cnt, INDEX right_cnt) {
        using G = pairwise_local_alignment_graph<
                std::tuple<>,
                ED,
                INDEX,
                offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<std::tuple<>, INDEX>,
                offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ED, INDEX>,
                error_check
            >;
        return created_graph<G> {
            G {
                down_cnt,
                right_cnt
            }
        };
    }

    TEST(ReversedPairwiseAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_local_alignment_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph<G>);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, ListNodes) {
        auto x = [](auto&& g) {
            auto n = g.get_nodes();
            EXPECT_EQ(
                std::set(n.begin(), n.end()),
                (std::set {
                    std::pair{0u, 0u}, std::pair{0u, 1u}, std::pair{0u, 2u},
                    std::pair{1u, 0u}, std::pair{1u, 1u}, std::pair{1u, 2u}
                })
            );
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, ListEdges) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            auto e = g.get_edges();
            std::multiset<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : e) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::multiset<E> {
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 2u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 2u} } },

                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{0u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{1u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} } },

                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 2u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{1u, 0u}, std::pair{1u, 2u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} } }
                })
            );
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, NodesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_node({0u, 0u}));
            EXPECT_TRUE(g.has_node({0u, 1u}));
            EXPECT_TRUE(g.has_node({0u, 2u}));
            EXPECT_FALSE(g.has_node({0u, 3u}));
            EXPECT_TRUE(g.has_node({1u, 0u}));
            EXPECT_TRUE(g.has_node({1u, 1u}));
            EXPECT_TRUE(g.has_node({1u, 2u}));
            EXPECT_FALSE(g.has_node({1u, 3u}));
            EXPECT_FALSE(g.has_node({2u, 3u}));
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 0u}, {0u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 1u}, {0u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0u, 2u}, {0u, 3u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{1u, 0u}, {1u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{1u, 1u}, {1u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 2u}, {1u, 3u}}}));
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, DownEdgesExist) {
        auto x = [](auto&& g) {

            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 0u}, {1u, 0u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 0u}, {2u, 0u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 1u}, {1u, 1u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 1u}, {2u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 2u}, {1u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 2u}, {2u, 2u}}}));
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 0u}, {1u, 1u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 0u}, {2u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 1u}, {1u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 1u}, {2u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0u, 2u}, {1u, 3u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 2u}, {2u, 3u}}}));
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, FreeRideEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 0u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 2u} } }));

            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1u, 0u}, std::pair{1u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1u, 2u}, std::pair{1u, 2u} } }));
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetOutputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 1u, 2u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                std::vector<E> expected {
                    { edge_type::FREE_RIDE, { {0u, 0u}, {1u, 2u} } },
                    { edge_type::FREE_RIDE, { {0u, 1u}, {1u, 2u} } },
                    { edge_type::FREE_RIDE, { {0u, 2u}, {1u, 2u} } },
                    { edge_type::FREE_RIDE, { {1u, 0u}, {1u, 2u} } },
                    { edge_type::FREE_RIDE, { {1u, 1u}, {1u, 2u} } },
                    { edge_type::NORMAL, { {0u, 1u}, {1u, 2u} } },
                    { edge_type::NORMAL, { {0u, 2u}, {1u, 2u} } },
                    { edge_type::NORMAL, { {1u, 1u}, {1u, 2u} } }
                };
                EXPECT_EQ(actual, expected);
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 0u, 0u } )) {
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
                for (auto _e : g.get_outputs( { 1u, 1u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, {{0u, 0u}, {1u, 1u} } },
                        { edge_type::NORMAL, { {0u, 0u}, {1u, 1u} } },
                        { edge_type::NORMAL, { {0u, 1u}, {1u, 1u} } },
                        { edge_type::NORMAL, { {1u, 0u}, {1u, 1u} } }
                    })
                );
            }
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetInputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{ 1u, 2u } )) {
                    actual.push_back(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {})
                );
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( { 0u, 0u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, {{0u, 0u}, {0u, 1u} } },
                        { edge_type::FREE_RIDE, {{0u, 0u}, {0u, 2u} } },
                        { edge_type::FREE_RIDE, {{0u, 0u}, {1u, 0u} } },
                        { edge_type::FREE_RIDE, {{0u, 0u}, {1u, 1u} } },
                        { edge_type::FREE_RIDE, {{0u, 0u}, {1u, 2u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {0u, 1u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {1u, 0u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {1u, 1u} } }
                    })
                );
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{0u, 1u} )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, { {0u, 1u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {0u, 1u}, {0u, 2u} } },
                        { edge_type::NORMAL, { {0u, 1u}, {1u, 1u} } },
                        { edge_type::NORMAL, { {0u, 1u}, {1u, 2u} } }
                    })
                );
            }
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetOutputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 0u } ), 0);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 2u } ), 8);
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 2u } ), 2);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 2);
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }

    TEST(ReversedPairwiseAlignmentGraphTest, GetInputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 0u } ), 8);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 2u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 2);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 2);
        };
        x(create_vector<float>(2u, 3u).reversed_g);
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

        auto x = [&](auto&& g) {
            using G = std::decay_t<decltype(g)>;
            using N = typename G::N;
            using E = typename G::E;

            EXPECT_EQ(G::slice_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 3zu);
            EXPECT_EQ(g.slice_first_node(0u), (N { 1u, 2u }));
            EXPECT_EQ(g.slice_first_node(0u, 0u), (N { 1u, 2u }));
            EXPECT_EQ(g.slice_first_node(0u, 1u), (N { 1u, 1u }));
            EXPECT_EQ(g.slice_first_node(0u, 2u), (N { 1u, 0u }));
            EXPECT_EQ(g.slice_last_node(0u), (N { 1u, 0u }));
            EXPECT_EQ(g.slice_last_node(0u, 2u), (N { 1u, 0u }));
            EXPECT_EQ(g.slice_last_node(0u, 1u), (N { 1u, 1u }));
            EXPECT_EQ(g.slice_last_node(0u, 0u), (N { 1u, 2u }));
            EXPECT_EQ(g.slice_first_node(1u), (N { 0u, 2u }));
            EXPECT_EQ(g.slice_last_node(1u), (N { 0u, 0u }));

            EXPECT_EQ(G::resident_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 2zu);
            EXPECT_EQ(g.resident_nodes().size(), 2zu);
            EXPECT_EQ(g.resident_nodes()[0], (N { 0u, 0u }));
            EXPECT_EQ(g.resident_nodes()[1], (N { 1u, 2u }));

            EXPECT_EQ(g.slice_prev_node(N { 0u, 0u }), (N { 0u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { 0u, 1u }), (N { 0u, 2u }));
            EXPECT_EQ(g.slice_prev_node(N { 1u, 0u }), (N { 1u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { 1u, 1u }), (N { 1u, 2u }));

            EXPECT_EQ(g.slice_next_node(N { 0u, 1u }), (N { 0u, 0u }));
            EXPECT_EQ(g.slice_next_node(N { 0u, 2u }), (N { 0u, 1u }));
            EXPECT_EQ(g.slice_next_node(N { 1u, 1u }), (N { 1u, 0u }));
            EXPECT_EQ(g.slice_next_node(N { 1u, 2u }), (N { 1u, 1u }));

            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 0u, 0u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 0u, 1u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 1u }, { 1u, 2u } } },
                    E { edge_type::NORMAL, { { 0u, 1u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 0u, 2u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 2u }, { 1u, 2u } } },
                    E { edge_type::NORMAL, { { 0u, 2u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 1u, 0u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 1u, 0u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 1u, 1u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 1u, 1u }, { 1u, 2u } } },
                    E { edge_type::NORMAL, { { 1u, 1u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 1u, 2u })),
                (std::vector<E> {})
            );


            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 0u, 0u })),
                (std::vector<E> {})
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 0u, 1u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 0u, 1u } } },
                    E { edge_type::NORMAL, { { 0u, 0u }, { 0u, 1u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 0u, 2u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 0u, 2u } } },
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 1u, 0u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 1u, 0u } } },
                    E { edge_type::NORMAL, { { 0u, 0u }, { 1u, 0u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 1u, 1u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 1u, 1u } } },
                    E { edge_type::NORMAL, { { 0u, 0u }, { 1u, 1u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 1u, 2u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 1u, 2u } } }
                })
            );
        };
        x(create_vector<float>(2u, 3u).reversed_g);
    }
}