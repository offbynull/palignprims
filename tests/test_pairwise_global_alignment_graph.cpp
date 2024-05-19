#include "pairwise_global_alignment_graph.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace offbynull::pairwise_aligner::global;

namespace {
    template<typename _ED, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_vector(T down_cnt, T right_cnt) {
        return pairwise_global_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::VectorAllocator<node_data<_ED, T>, T>,
            offbynull::graph::graph_helpers::VectorAllocator<_ED, T>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_array() {
        return pairwise_global_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::ArrayAllocator<node_data<_ED, T>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::graph::graph_helpers::ArrayAllocator<_ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_small_vector(T down_cnt, T right_cnt) {
        return pairwise_global_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::StaticVectorAllocator<node_data<_ED, T>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::graph::graph_helpers::StaticVectorAllocator<_ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
    requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_static_vector(T down_cnt, T right_cnt) {
        return pairwise_global_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::SmallVectorAllocator<node_data<_ED, T>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            offbynull::graph::graph_helpers::SmallVectorAllocator<_ED, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(PairwiseGlobalAlignmentGraphTest, ListNodes) {
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

    TEST(PairwiseGlobalAlignmentGraphTest, ListEdges) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, NodesExist) {
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

    TEST(PairwiseGlobalAlignmentGraphTest, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {0u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {0u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {0u, 3u}}));
            EXPECT_TRUE(g.has_edge({{1u, 0u}, {1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{1u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {1u, 3u}}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, DownEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {1u, 0u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 0u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 2u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 2u}}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {1u, 3u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 3u}}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, GetOutputs) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, GetInputs) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, GetOutputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 0u } ), 3);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 2u } ), 0);
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseGlobalAlignmentGraphTest, GetInputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 0u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 2u } ), 3);
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }
    /*
    TEST(PairwiseGlobalAlignmentGraphTest, BuildTest) {
        auto weight_lookup {
            [](const std::optional<std::reference_wrapper<const char>>& v_elem, const std::optional<std::reference_wrapper<const char>>& w_elem) {
                if (v_elem == std::nullopt || w_elem == std::nullopt) {
                    return -1.0;
                } else if ((*v_elem).get() == (*w_elem).get()) {
                    return 1.0;
                } else {
                    return 0.0;
                }
            }
        };
        auto x {
            [](auto&& g) {
                EXPECT_EQ(g.get_edge_data({{ 0u, 0u }, { 0u, 1u }}), -1.0);  // a and -
                EXPECT_EQ(g.get_edge_data({{ 0u, 0u }, { 1u, 0u }}), -1.0);  // - and b
                EXPECT_EQ(g.get_edge_data({{ 0u, 0u }, { 1u, 1u }}), 0.0);   // a and b
                EXPECT_EQ(g.get_edge_data({{ 1u, 0u }, { 1u, 1u }}), -1.0);  // b and -
                EXPECT_EQ(g.get_edge_data({{ 1u, 0u }, { 2u, 0u }}), -1.0);  // - and b
                EXPECT_EQ(g.get_edge_data({{ 1u, 0u }, { 2u, 1u }}), 1.0);   // b and b
            }
        };
        x(create_vector_and_assign<double, char>(std::string { "ab" }, std::string { "b" }, weight_lookup));
        x(create_array_and_assign<double, char, 3u, 2u>(std::string { "ab" }, std::string { "b" }, weight_lookup));
        x(create_small_vector_and_assign<double, char, 3u, 2u>(std::string { "ab" }, std::string { "b" }, weight_lookup));
        x(create_static_vector_and_assign<double, char, 3u, 2u>(std::string { "ab" }, std::string { "b" }, weight_lookup));
    }
    */
}