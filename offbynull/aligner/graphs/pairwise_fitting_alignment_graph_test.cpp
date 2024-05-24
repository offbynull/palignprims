#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/grid_allocators.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge_type;

    template<typename ED, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_vector(T down_cnt, T right_cnt) {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::VectorGridAllocator<std::tuple<>, T>,
            offbynull::aligner::graph::grid_allocators::VectorGridAllocator<ED, T>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_array() {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::ArrayGridAllocator<std::tuple<>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_allocators::ArrayGridAllocator<ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT
        };
    }

    template<typename ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_small_vector(T down_cnt, T right_cnt) {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::StaticVectorGridAllocator<std::tuple<>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_allocators::StaticVectorGridAllocator<ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
    requires std::is_floating_point_v<ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_static_vector(T down_cnt, T right_cnt) {
        return pairwise_fitting_alignment_graph<
            std::tuple<>,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::SmallVectorGridAllocator<std::tuple<>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_allocators::SmallVectorGridAllocator<ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(PairwiseFittingAlignmentGraph, ConceptCheck) {
        using G = pairwise_fitting_alignment_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
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
            std::multiset<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : e) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::multiset<E> {
                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } },

                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{0u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{1u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} } },
                    edge { edge_type::NORMAL, std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} } },

                    edge { edge_type::FREE_RIDE, std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} } }
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
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 0u, 0u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        { edge_type::NORMAL, {{0u, 0u}, {0u, 1u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {1u, 0u} } },
                        { edge_type::NORMAL, {{0u, 0u}, {1u, 1u} } },
                        { edge_type::FREE_RIDE, {{0u, 0u}, {1u, 2u} } }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 1u, 2u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {})
                );
            }
            {
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 0u, 2u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        { edge_type::NORMAL, { {0u, 2u}, {1u, 2u} } },
                        { edge_type::FREE_RIDE, {{0u, 2u}, {1u, 2u} } }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_outputs( { 1u, 0u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        { edge_type::NORMAL, { {1u, 0u}, {1u, 1u} } },
                        { edge_type::FREE_RIDE, { {1u, 0u}, {1u, 2u} } }
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
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{ 0u, 0u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {})
                );
            }
            {
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{ 1u, 2u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        { edge_type::NORMAL, { {0u, 2u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {1u, 1u}, {1u, 2u} } },
                        { edge_type::NORMAL, { {0u, 1u}, {1u, 2u} } },
                        { edge_type::FREE_RIDE, { {0u, 0u}, {1u, 2u} } }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{0u, 2u} )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        { edge_type::NORMAL, { {0u, 1u}, {0u, 2u} } },
                        { edge_type::FREE_RIDE, { {0u, 0u}, {0u, 2u} } }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
                for (auto _e : g.get_inputs( std::pair{1u, 0u} )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        { edge_type::NORMAL, { {0u, 0u}, {1u, 0u} } },
                        { edge_type::FREE_RIDE, { {0u, 0u}, {1u, 0u} } }
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
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 2);
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
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 2);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 2);
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }
}