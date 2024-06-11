#include <cstddef>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge_type;

    template<typename ED, typename INDEX = unsigned int, bool error_check = true>
    auto create_vector(INDEX down_cnt, INDEX right_cnt) {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<std::tuple<>, INDEX>,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ED, INDEX>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ED, std::size_t STATIC_DOWN_CNT, std::size_t STATIC_RIGHT_CNT, typename INDEX = unsigned int, bool error_check = true>
    auto create_array() {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::array_grid_container_creator<std::tuple<>, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_container_creators::array_grid_container_creator<ED, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT
        };
    }

    template<typename ED, std::size_t STATIC_DOWN_CNT, std::size_t STATIC_RIGHT_CNT, typename INDEX = unsigned int, bool error_check = true>
    auto create_small_vector(INDEX down_cnt, INDEX right_cnt) {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::static_vector_grid_container_creator<std::tuple<>, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_container_creators::static_vector_grid_container_creator<ED, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ED, std::size_t STATIC_DOWN_CNT, std::size_t STATIC_RIGHT_CNT, typename INDEX = unsigned int, bool error_check = true>
    auto create_static_vector(INDEX down_cnt, INDEX right_cnt) {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::small_vector_grid_container_creator<std::tuple<>, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_container_creators::small_vector_grid_container_creator<ED, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(PairwiseFittingAlignmentGraph, ConceptCheck) {
        using G = pairwise_fitting_alignment_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph<G>);
    }

    TEST(PairwiseFittingAlignmentGraph, ListNodes) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, ListEdges) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            auto e = g.get_edges();
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : e) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } },
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{0u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{1u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} } }
                })
            );
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, NodesExist) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 0u}, {0u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 1u}, {0u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0u, 2u}, {0u, 3u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{1u, 0u}, {1u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{1u, 1u}, {1u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 2u}, {1u, 3u}}}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, DownEdgesExist) {
        auto x = [](auto&& g) {

            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 0u}, {1u, 0u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 0u}, {2u, 0u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 1u}, {1u, 1u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 1u}, {2u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 2u}, {1u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 2u}, {2u, 2u}}}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 0u}, {1u, 1u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 0u}, {2u, 1u}}}));
            EXPECT_TRUE(g.has_edge({edge_type::NORMAL, {{0u, 1u}, {1u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 1u}, {2u, 2u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0u, 2u}, {1u, 3u}}}));
            EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{1u, 2u}, {2u, 3u}}}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, FreeRideEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 0u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{0u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 2u} } }));

            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 2u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} } }));
            EXPECT_TRUE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1u, 0u}, std::pair{1u, 2u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} } }));
            EXPECT_FALSE(g.has_edge({ edge_type::FREE_RIDE, std::pair { std::pair{1u, 2u}, std::pair{1u, 2u} } }));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, GetOutputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 0u, 0u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, {{0u, 0u}, {1u, 0u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {0u, 1u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {1u, 0u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {1u, 1u} } }
                    })
                );
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 1u, 2u } )) {
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
                for (auto _e : g.get_outputs( { 0u, 2u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, {{0u, 2u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {0u, 2u}, {1u, 2u} } }
                    })
                );
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 1u, 0u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::NORMAL, { {1u, 0u}, {1u, 1u} } },
                    })
                );
            }
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, GetInputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{ 0u, 0u } )) {
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
                for (auto _e : g.get_inputs( std::pair{ 1u, 2u } )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, { {0u, 2u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {0u, 1u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {0u, 2u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {1u, 1u}, {1u, 2u} } }
                    })
                );
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{0u, 2u} )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::NORMAL, { {0u, 1u}, {0u, 2u} } }
                    })
                );
            }
            {
                std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{1u, 0u} )) {
                    actual.push_back(_e);
                }
                std::sort(actual.begin(), actual.end());
                EXPECT_EQ(
                    actual,
                    (std::vector<E> {
                        { edge_type::FREE_RIDE, { {0u, 0u}, {1u, 0u} } },
                        { edge_type::NORMAL, { {0u, 0u}, {1u, 0u} } }
                    })
                );
            }
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, GetOutputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 0u } ), 4);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 2u } ), 0);
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 2u } ), 2);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, GetInputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 0u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 2u } ), 4);
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 2);
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseFittingAlignmentGraph, SlicedWalk) {
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
            using N = typename std::remove_reference_t<decltype(g)>::N;
            using E = typename std::remove_reference_t<decltype(g)>::E;

            EXPECT_EQ(g.max_slice_nodes_count(), 3zu);
            EXPECT_EQ(g.first_node_in_slice(0u), (N { 0u, 0u }));
            EXPECT_EQ(g.last_node_in_slice(0u), (N { 0u, 2u }));
            EXPECT_EQ(g.first_node_in_slice(1u), (N { 1u, 0u }));
            EXPECT_EQ(g.last_node_in_slice(1u), (N {  1u, 2u }));

            EXPECT_EQ(g.max_resident_nodes_count(), 2zu);
            EXPECT_EQ(g.resident_nodes().size(), 2zu);
            EXPECT_EQ(g.resident_nodes()[0], (N { 0u, 0u }));
            EXPECT_EQ(g.resident_nodes()[1], (N { 1u, 2u }));

            EXPECT_EQ(g.next_node_in_slice(N { 0u, 0u }), (N { 0u, 1u }));
            EXPECT_EQ(g.next_node_in_slice(N { 0u, 1u }), (N { 0u, 2u }));
            EXPECT_EQ(g.next_node_in_slice(N { 1u, 0u }), (N { 1u, 1u }));
            EXPECT_EQ(g.next_node_in_slice(N { 1u, 1u }), (N { 1u, 2u }));

            EXPECT_EQ(g.prev_node_in_slice(N { 0u, 1u }), (N { 0u, 0u }));
            EXPECT_EQ(g.prev_node_in_slice(N { 0u, 2u }), (N { 0u, 1u }));
            EXPECT_EQ(g.prev_node_in_slice(N { 1u, 1u }), (N { 1u, 0u }));
            EXPECT_EQ(g.prev_node_in_slice(N { 1u, 2u }), (N { 1u, 1u }));

            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 0u, 0u })),
                (std::vector<E> {})
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 0u, 1u })),
                (std::vector<E> {
                    E { edge_type::NORMAL, { { 0u, 1u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 0u, 2u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 2u }, { 1u, 2u } } },
                    E { edge_type::NORMAL, { { 0u, 2u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 1u, 0u })),
                (std::vector<E> {})
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 1u, 1u })),
                (std::vector<E> {
                    E { edge_type::NORMAL, { { 1u, 1u }, { 1u, 2u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.outputs_to_residents(N { 1u, 2u })),
                (std::vector<E> {})
            );


            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 0u, 0u })),
                (std::vector<E> {})
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 0u, 1u })),
                (std::vector<E> {
                    E { edge_type::NORMAL, { { 0u, 0u }, { 0u, 1u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 0u, 2u })),
                (std::vector<E> {})
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 1u, 0u })),
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0u, 0u }, { 1u, 0u } } },
                    E { edge_type::NORMAL, { { 0u, 0u }, { 1u, 0u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 1u, 1u })),
                (std::vector<E> {
                    E { edge_type::NORMAL, { { 0u, 0u }, { 1u, 1u } } }
                })
            );
            EXPECT_EQ(
                to_vector(g.inputs_from_residents(N { 1u, 2u })),
                (std::vector<E> {})
            );
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }
}