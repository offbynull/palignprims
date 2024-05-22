#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/grid_allocators.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::grid_graph::grid_graph;

    template<typename ND, typename ED, typename T = unsigned int, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_vector(T down_cnt, T right_cnt) {
        return grid_graph<
            ND,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::VectorAllocator<ND, T>,
            offbynull::aligner::graph::grid_allocators::VectorAllocator<ED, T>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ND, typename ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_array() {
        return grid_graph<
            ND,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::ArrayAllocator<ND, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_allocators::ArrayAllocator<ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT
        };
    }

    template<typename ND, typename ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_small_vector(T down_cnt, T right_cnt) {
        return grid_graph<
            ND,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::StaticVectorAllocator<ND, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_allocators::StaticVectorAllocator<ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ND, typename ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_static_vector(T down_cnt, T right_cnt) {
        return grid_graph<
            ND,
            ED,
            T,
            offbynull::aligner::graph::grid_allocators::SmallVectorAllocator<ND, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::aligner::graph::grid_allocators::SmallVectorAllocator<ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(GridGraphTest, ConceptCheck) {
        using G = grid_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
    }

    TEST(GridGraphTest, ListNodes) {
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
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, ListEdges) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            auto e = g.get_edges();
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : e) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} },
                    std::pair { std::pair{0u, 1u}, std::pair{0u, 2u} },
                    std::pair { std::pair{1u, 0u}, std::pair{1u, 1u} },
                    std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} },
                    std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} },
                    std::pair { std::pair{0u, 1u}, std::pair{1u, 1u} },
                    std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} },
                    std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} },
                    std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} }
                })
            );
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, NodesExist) {
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
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {0u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {0u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {0u, 3u}}));
            EXPECT_TRUE(g.has_edge({{1u, 0u}, {1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{1u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {1u, 3u}}));
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, DownEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {1u, 0u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 0u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 2u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 2u}}));
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {1u, 3u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 3u}}));
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, GetOutputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_outputs( std::pair{ 0u, 0u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} },
                        std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} },
                        std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_outputs( std::pair{ 1u, 2u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {})
                );
            }
            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_outputs( std::pair{ 0u, 2u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_outputs( std::pair{ 1u, 0u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        std::pair { std::pair{1u, 0u}, std::pair{1u, 1u} }
                    })
                );
            }
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, GetInputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;

            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_inputs( std::pair{ 0u, 0u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {})
                );
            }
            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_inputs( std::pair{ 1u, 2u } )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} },
                        std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} },
                        std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_inputs( std::pair{0u, 2u} )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        std::pair { std::pair{0u, 1u}, std::pair{0u, 2u} }
                    })
                );
            }
            {
                std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                for (auto _e : g.get_inputs( std::pair{1u, 0u} )) {
                    actual.insert(_e);
                }
                EXPECT_EQ(
                    actual,
                    (std::set<E> {
                        std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} }
                    })
                );
            }
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, GetOutputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 0u } ), 3);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 2u } ), 0);
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphTest, GetInputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 0u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 2u } ), 3);
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }
}