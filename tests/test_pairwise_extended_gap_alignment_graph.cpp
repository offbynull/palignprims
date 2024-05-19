#include "pairwise_extended_gap_alignment_graph.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace offbynull::pairwise_aligner::extended_gap;

namespace {
    template<typename _ED, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_vector(T down_cnt, T right_cnt) {
        return pairwise_extended_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::VectorAllocator<slot<_ED, T>, T>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_array() {
        return pairwise_extended_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::ArrayAllocator<slot<_ED, T>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_small_vector(T down_cnt, T right_cnt) {
        return pairwise_extended_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::StaticVectorAllocator<slot<_ED, T>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
    requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_static_vector(T down_cnt, T right_cnt) {
        return pairwise_extended_alignment_graph<
            _ED,
            T,
            offbynull::graph::graph_helpers::SmallVectorAllocator<slot<_ED, T>, T, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ListNodes) {
        auto x = [](auto&& g) {
            using N = typename std::remove_reference_t<decltype(g)>::N;

            std::set<N> actual {};
            for (const auto &n : g.get_nodes()) {
                actual.insert(n);
            }
            EXPECT_EQ(
                actual,
                (std::set {
                    N {layer::DIAGONAL, 0u, 0u},
                    N {layer::DIAGONAL, 0u, 1u},
                    N {layer::DIAGONAL, 0u, 2u},
                    N {layer::DIAGONAL, 0u, 3u},
                    N {layer::DIAGONAL, 1u, 0u},
                    N {layer::DIAGONAL, 1u, 1u},
                    N {layer::DIAGONAL, 1u, 2u},
                    N {layer::DIAGONAL, 1u, 3u},
                    N {layer::DIAGONAL, 2u, 0u},
                    N {layer::DIAGONAL, 2u, 1u},
                    N {layer::DIAGONAL, 2u, 2u},
                    N {layer::DIAGONAL, 2u, 3u},
                    N {layer::DOWN, 1u, 0u},
                    N {layer::DOWN, 1u, 1u},
                    N {layer::DOWN, 1u, 2u},
                    N {layer::DOWN, 1u, 3u},
                    N {layer::DOWN, 2u, 0u},
                    N {layer::DOWN, 2u, 1u},
                    N {layer::DOWN, 2u, 2u},
                    N {layer::DOWN, 2u, 3u},
                    N {layer::RIGHT, 0u, 1u},
                    N {layer::RIGHT, 1u, 1u},
                    N {layer::RIGHT, 2u, 1u},
                    N {layer::RIGHT, 0u, 2u},
                    N {layer::RIGHT, 1u, 2u},
                    N {layer::RIGHT, 2u, 2u},
                    N {layer::RIGHT, 0u, 3u},
                    N {layer::RIGHT, 1u, 3u},
                    N {layer::RIGHT, 2u, 3u}
                })
            );
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ListEdges) {
        auto x = [](auto&& g) {
            using N = typename std::remove_reference_t<decltype(g)>::N;
            using E = typename std::remove_reference_t<decltype(g)>::E;

            auto e = g.get_edges();
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : e) {
                actual.insert(_e);
            }
            std::set expected {
                std::set<E> {
                    E { N {layer::DIAGONAL, 0u, 0u}, N {layer::DIAGONAL, 1u, 1u} },
                    E { N {layer::DIAGONAL, 0u, 1u}, N {layer::DIAGONAL, 1u, 2u} },
                    E { N {layer::DIAGONAL, 0u, 2u}, N {layer::DIAGONAL, 1u, 3u} },
                    E { N {layer::DIAGONAL, 1u, 0u}, N {layer::DIAGONAL, 2u, 1u} },
                    E { N {layer::DIAGONAL, 1u, 1u}, N {layer::DIAGONAL, 2u, 2u} },
                    E { N {layer::DIAGONAL, 1u, 2u}, N {layer::DIAGONAL, 2u, 3u} },
                    E { N {layer::DIAGONAL, 0u, 0u}, N {layer::RIGHT, 0u, 1u} },
                    E { N {layer::DIAGONAL, 0u, 1u}, N {layer::RIGHT, 0u, 2u} },
                    E { N {layer::DIAGONAL, 0u, 2u}, N {layer::RIGHT, 0u, 3u} },
                    E { N {layer::DIAGONAL, 1u, 0u}, N {layer::RIGHT, 1u, 1u} },
                    E { N {layer::DIAGONAL, 1u, 1u}, N {layer::RIGHT, 1u, 2u} },
                    E { N {layer::DIAGONAL, 1u, 2u}, N {layer::RIGHT, 1u, 3u} },
                    E { N {layer::DIAGONAL, 2u, 0u}, N {layer::RIGHT, 2u, 1u} },
                    E { N {layer::DIAGONAL, 2u, 1u}, N {layer::RIGHT, 2u, 2u} },
                    E { N {layer::DIAGONAL, 2u, 2u}, N {layer::RIGHT, 2u, 3u} },
                    E { N {layer::DIAGONAL, 0u, 0u}, N {layer::DOWN, 1u, 0u} },
                    E { N {layer::DIAGONAL, 1u, 0u}, N {layer::DOWN, 2u, 0u} },
                    E { N {layer::DIAGONAL, 0u, 1u}, N {layer::DOWN, 1u, 1u} },
                    E { N {layer::DIAGONAL, 1u, 1u}, N {layer::DOWN, 2u, 1u} },
                    E { N {layer::DIAGONAL, 0u, 2u}, N {layer::DOWN, 1u, 2u} },
                    E { N {layer::DIAGONAL, 1u, 2u}, N {layer::DOWN, 2u, 2u} },
                    E { N {layer::DIAGONAL, 0u, 3u}, N {layer::DOWN, 1u, 3u} },
                    E { N {layer::DIAGONAL, 1u, 3u}, N {layer::DOWN, 2u, 3u} },
                    E { N {layer::DOWN, 1u, 0u}, N {layer::DIAGONAL, 1u, 0u} },
                    E { N {layer::DOWN, 1u, 1u}, N {layer::DIAGONAL, 1u, 1u} },
                    E { N {layer::DOWN, 1u, 2u}, N {layer::DIAGONAL, 1u, 2u} },
                    E { N {layer::DOWN, 1u, 3u}, N {layer::DIAGONAL, 1u, 3u} },
                    E { N {layer::DOWN, 2u, 0u}, N {layer::DIAGONAL, 2u, 0u} },
                    E { N {layer::DOWN, 2u, 1u}, N {layer::DIAGONAL, 2u, 1u} },
                    E { N {layer::DOWN, 2u, 2u}, N {layer::DIAGONAL, 2u, 2u} },
                    E { N {layer::DOWN, 2u, 3u}, N {layer::DIAGONAL, 2u, 3u} },
                    E { N {layer::DOWN, 1u, 0u}, N {layer::DOWN, 2u, 0u} },
                    E { N {layer::DOWN, 1u, 1u}, N {layer::DOWN, 2u, 1u} },
                    E { N {layer::DOWN, 1u, 2u}, N {layer::DOWN, 2u, 2u} },
                    E { N {layer::DOWN, 1u, 3u}, N {layer::DOWN, 2u, 3u} },
                    E { N {layer::RIGHT, 0u, 1u}, N {layer::DIAGONAL, 0u, 1u} },
                    E { N {layer::RIGHT, 0u, 2u}, N {layer::DIAGONAL, 0u, 2u} },
                    E { N {layer::RIGHT, 0u, 3u}, N {layer::DIAGONAL, 0u, 3u} },
                    E { N {layer::RIGHT, 1u, 1u}, N {layer::DIAGONAL, 1u, 1u} },
                    E { N {layer::RIGHT, 1u, 2u}, N {layer::DIAGONAL, 1u, 2u} },
                    E { N {layer::RIGHT, 1u, 3u}, N {layer::DIAGONAL, 1u, 3u} },
                    E { N {layer::RIGHT, 2u, 1u}, N {layer::DIAGONAL, 2u, 1u} },
                    E { N {layer::RIGHT, 2u, 2u}, N {layer::DIAGONAL, 2u, 2u} },
                    E { N {layer::RIGHT, 2u, 3u}, N {layer::DIAGONAL, 2u, 3u} },
                    E { N {layer::RIGHT, 0u, 1u}, N {layer::RIGHT, 0u, 2u} },
                    E { N {layer::RIGHT, 0u, 2u}, N {layer::RIGHT, 0u, 3u} },
                    E { N {layer::RIGHT, 1u, 1u}, N {layer::RIGHT, 1u, 2u} },
                    E { N {layer::RIGHT, 1u, 2u}, N {layer::RIGHT, 1u, 3u} },
                    E { N {layer::RIGHT, 2u, 1u}, N {layer::RIGHT, 2u, 2u} },
                    E { N {layer::RIGHT, 2u, 2u}, N {layer::RIGHT, 2u, 3u} },
                }
            };
            EXPECT_EQ(
                actual,
                expected
            );
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, NodesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0u, 0u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0u, 1u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0u, 2u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0u, 3u}));
            EXPECT_FALSE(g.has_node({layer::DIAGONAL, 0u, 4u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1u, 0u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1u, 1u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1u, 2u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1u, 3u}));
            EXPECT_FALSE(g.has_node({layer::DIAGONAL, 1u, 4u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2u, 0u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2u, 1u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2u, 2u}));
            EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2u, 3u}));
            EXPECT_FALSE(g.has_node({layer::DIAGONAL, 2u, 4u}));
            EXPECT_FALSE(g.has_node({layer::DIAGONAL, 3u, 3u}));

            EXPECT_FALSE(g.has_node({layer::DOWN, 0u, 0u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 0u, 1u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 0u, 2u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 0u, 3u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 0u, 4u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 1u, 0u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 1u, 1u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 1u, 2u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 1u, 3u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 1u, 4u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 2u, 0u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 2u, 1u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 2u, 2u}));
            EXPECT_TRUE(g.has_node({layer::DOWN, 2u, 3u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 2u, 4u}));
            EXPECT_FALSE(g.has_node({layer::DOWN, 3u, 3u}));

            EXPECT_FALSE(g.has_node({layer::RIGHT, 0u, 0u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 0u, 1u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 0u, 2u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 0u, 3u}));
            EXPECT_FALSE(g.has_node({layer::RIGHT, 0u, 4u}));
            EXPECT_FALSE(g.has_node({layer::RIGHT, 1u, 0u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 1u, 1u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 1u, 2u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 1u, 3u}));
            EXPECT_FALSE(g.has_node({layer::RIGHT, 1u, 4u}));
            EXPECT_FALSE(g.has_node({layer::RIGHT, 2u, 0u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 2u, 1u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 2u, 2u}));
            EXPECT_TRUE(g.has_node({layer::RIGHT, 2u, 3u}));
            EXPECT_FALSE(g.has_node({layer::RIGHT, 2u, 4u}));
            EXPECT_FALSE(g.has_node({layer::RIGHT, 3u, 3u}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 2u}, {layer::RIGHT, 0u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 0u, 3u}, {layer::RIGHT, 0u, 4u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 0u}, {layer::RIGHT, 1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 1u}, {layer::RIGHT, 1u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 2u}, {layer::RIGHT, 1u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 1u, 3u}, {layer::RIGHT, 1u, 4u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 2u, 0u}, {layer::RIGHT, 2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 2u, 1u}, {layer::RIGHT, 2u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 2u, 2u}, {layer::RIGHT, 2u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2u, 3u}, {layer::RIGHT, 2u, 4u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 3u, 0u}, {layer::RIGHT, 3u, 1u}}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 0u}, {layer::DOWN, 2u, 0u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2u, 0u}, {layer::DOWN, 3u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 1u}, {layer::DOWN, 1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 1u}, {layer::DOWN, 2u, 1u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2u, 1u}, {layer::DOWN, 3u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 2u}, {layer::DOWN, 1u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 2u}, {layer::DOWN, 2u, 2u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2u, 2u}, {layer::DOWN, 3u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0u, 3u}, {layer::DOWN, 1u, 3u}}));
            EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1u, 3u}, {layer::DOWN, 2u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2u, 3u}, {layer::DOWN, 3u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 3u, 0u}, {layer::DOWN, 4u, 0u}}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u} }));
            EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 0u, 1u}, {layer::DIAGONAL, 1u, 2u} }));
            EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 0u, 2u}, {layer::DIAGONAL, 1u, 3u} }));
            EXPECT_FALSE(g.has_edge({ {layer::DIAGONAL, 0u, 3u}, {layer::DIAGONAL, 1u, 4u} }));
            EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 1u, 0u}, {layer::DIAGONAL, 2u, 1u} }));
            EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 1u, 1u}, {layer::DIAGONAL, 2u, 2u} }));
            EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 1u, 2u}, {layer::DIAGONAL, 2u, 3u} }));
            EXPECT_FALSE(g.has_edge({ {layer::DIAGONAL, 1u, 3u}, {layer::DIAGONAL, 2u, 4u} }));
            EXPECT_FALSE(g.has_edge({ {layer::DIAGONAL, 2u, 0u}, {layer::DIAGONAL, 3u, 1u} }));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownFreeRidgeEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 0u, 0u}, {layer::DIAGONAL, 0u, 0u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 0u, 2u}, {layer::DIAGONAL, 0u, 2u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 0u, 3u}, {layer::DIAGONAL, 0u, 3u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 1u, 2u}, {layer::DIAGONAL, 1u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 1u, 3u}, {layer::DIAGONAL, 1u, 3u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 2u, 0u}, {layer::DIAGONAL, 2u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 2u, 1u}, {layer::DIAGONAL, 2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 2u, 2u}, {layer::DIAGONAL, 2u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::DOWN, 2u, 3u}, {layer::DIAGONAL, 2u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 3u, 0u}, {layer::DIAGONAL, 3u, 0u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 3u, 1u}, {layer::DIAGONAL, 3u, 1u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 3u, 2u}, {layer::DIAGONAL, 3u, 2u}}));
            EXPECT_FALSE(g.has_edge({{layer::DOWN, 3u, 3u}, {layer::DIAGONAL, 3u, 3u}}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightFreeRidgeEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 0u, 0u}, {layer::DIAGONAL, 0u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 0u, 2u}, {layer::DIAGONAL, 0u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 0u, 3u}, {layer::DIAGONAL, 0u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 1u, 2u}, {layer::DIAGONAL, 1u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 1u, 3u}, {layer::DIAGONAL, 1u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 2u, 0u}, {layer::DIAGONAL, 2u, 0u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 2u, 1u}, {layer::DIAGONAL, 2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 2u, 2u}, {layer::DIAGONAL, 2u, 2u}}));
            EXPECT_TRUE(g.has_edge({{layer::RIGHT, 2u, 3u}, {layer::DIAGONAL, 2u, 3u}}));
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3u, 0u}, {layer::DIAGONAL, 3u, 0u}}));
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3u, 1u}, {layer::DIAGONAL, 3u, 1u}}));
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3u, 2u}, {layer::DIAGONAL, 3u, 2u}}));
            EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3u, 3u}, {layer::DIAGONAL, 3u, 3u}}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;
            using N = typename std::remove_reference_t<decltype(g)>::N;
            auto to_outputs {
                [&](N n) {
                    std::set<E> x {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                    for (auto e : g.get_outputs(n)) {
                        x.insert(e);
                    }
                    return x;
                }
            };
            // Diagonal 0,0 to 0,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}},
                        {{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}},
                        {{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 0u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 1u}, {layer::DIAGONAL, 1u, 2u}},
                        {{layer::DIAGONAL, 0u, 1u}, {layer::DOWN, 1u, 1u}},
                        {{layer::DIAGONAL, 0u, 1u}, {layer::RIGHT, 0u, 2u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 0u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 2u}, {layer::DIAGONAL, 1u, 3u}},
                        {{layer::DIAGONAL, 0u, 2u}, {layer::DOWN, 1u, 2u}},
                        {{layer::DIAGONAL, 0u, 2u}, {layer::RIGHT, 0u, 3u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 0u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 3u}, {layer::DOWN, 1u, 3u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 0u, 3u})
            );
            // Diagonal 1,0 to 1,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 0u}, {layer::DIAGONAL, 2u, 1u}},
                        {{layer::DIAGONAL, 1u, 0u}, {layer::DOWN, 2u, 0u}},
                        {{layer::DIAGONAL, 1u, 0u}, {layer::RIGHT, 1u, 1u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 1u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 1u}, {layer::DIAGONAL, 2u, 2u}},
                        {{layer::DIAGONAL, 1u, 1u}, {layer::DOWN, 2u, 1u}},
                        {{layer::DIAGONAL, 1u, 1u}, {layer::RIGHT, 1u, 2u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 1u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 2u}, {layer::DIAGONAL, 2u, 3u}},
                        {{layer::DIAGONAL, 1u, 2u}, {layer::DOWN, 2u, 2u}},
                        {{layer::DIAGONAL, 1u, 2u}, {layer::RIGHT, 1u, 3u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 1u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 3u}, {layer::DOWN, 2u, 3u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 1u, 3u})
            );
            // Diagonal 2,0 to 2,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 2u, 0u}, {layer::RIGHT, 2u, 1u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 2u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 2u, 1u}, {layer::RIGHT, 2u, 2u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 2u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 2u, 2u}, {layer::RIGHT, 2u, 3u}}
                    }
                ),
                to_outputs({layer::DIAGONAL, 2u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                    }
                ),
                to_outputs({layer::DIAGONAL, 2u, 3u})
            );
            // Down 1,0 to 1,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}},
                        {{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}},
                    }
                ),
                to_outputs({layer::DOWN, 1u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}},
                        {{layer::DOWN, 1u, 1u}, {layer::DOWN, 2u, 1u}},
                    }
                ),
                to_outputs({layer::DOWN, 1u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 1u, 2u}, {layer::DIAGONAL, 1u, 2u}},
                        {{layer::DOWN, 1u, 2u}, {layer::DOWN, 2u, 2u}},
                    }
                ),
                to_outputs({layer::DOWN, 1u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 1u, 3u}, {layer::DIAGONAL, 1u, 3u}},
                        {{layer::DOWN, 1u, 3u}, {layer::DOWN, 2u, 3u}},
                    }
                ),
                to_outputs({layer::DOWN, 1u, 3u})
            );
            // Down 2,0 to 2,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 2u, 0u}, {layer::DIAGONAL, 2u, 0u}},
                    }
                ),
                to_outputs({layer::DOWN, 2u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 2u, 1u}, {layer::DIAGONAL, 2u, 1u}},
                    }
                ),
                to_outputs({layer::DOWN, 2u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 2u, 2u}, {layer::DIAGONAL, 2u, 2u}},
                    }
                ),
                to_outputs({layer::DOWN, 2u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 2u, 3u}, {layer::DIAGONAL, 2u, 3u}},
                    }
                ),
                to_outputs({layer::DOWN, 2u, 3u})
            );
            // Right 0,1 to 0,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}},
                        {{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}},
                    }
                ),
                to_outputs({layer::RIGHT, 0u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 0u, 2u}, {layer::DIAGONAL, 0u, 2u}},
                        {{layer::RIGHT, 0u, 2u}, {layer::RIGHT, 0u, 3u}},
                    }
                ),
                to_outputs({layer::RIGHT, 0u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 0u, 3u}, {layer::DIAGONAL, 0u, 3u}},
                    }
                ),
                to_outputs({layer::RIGHT, 0u, 3u})
            );
            // Right 1,1 to 1,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}},
                        {{layer::RIGHT, 1u, 1u}, {layer::RIGHT, 1u, 2u}},
                    }
                ),
                to_outputs({layer::RIGHT, 1u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 1u, 2u}, {layer::DIAGONAL, 1u, 2u}},
                        {{layer::RIGHT, 1u, 2u}, {layer::RIGHT, 1u, 3u}},
                    }
                ),
                to_outputs({layer::RIGHT, 1u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 1u, 3u}, {layer::DIAGONAL, 1u, 3u}},
                    }
                ),
                to_outputs({layer::RIGHT, 1u, 3u})
            );
            // Right 2,1 to 2,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 2u, 1u}, {layer::DIAGONAL, 2u, 1u}},
                        {{layer::RIGHT, 2u, 1u}, {layer::RIGHT, 2u, 2u}},
                    }
                ),
                to_outputs({layer::RIGHT, 2u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 2u, 2u}, {layer::DIAGONAL, 2u, 2u}},
                        {{layer::RIGHT, 2u, 2u}, {layer::RIGHT, 2u, 3u}},
                    }
                ),
                to_outputs({layer::RIGHT, 2u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 2u, 3u}, {layer::DIAGONAL, 2u, 3u}},
                    }
                ),
                to_outputs({layer::RIGHT, 2u, 3u})
            );
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputs) {
        auto x = [](auto&& g) {
            using E = typename std::remove_reference_t<decltype(g)>::E;
            using N = typename std::remove_reference_t<decltype(g)>::N;
            auto to_inputs {
                [&](N n) {
                    std::set<E> x {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                    for (auto e : g.get_inputs(n)) {
                        x.insert(e);
                    }
                    return x;
                }
            };
            // Diagonal 0,0 to 0,3
            EXPECT_EQ(
                (
                    std::set<E> {
                    }
                ),
                to_inputs({layer::DIAGONAL, 0u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 0u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 0u, 2u}, {layer::DIAGONAL, 0u, 2u}},
                    }
                ),
                to_inputs({layer::DIAGONAL, 0u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::RIGHT, 0u, 3u}, {layer::DIAGONAL, 0u, 3u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 0u, 3u})
            );
            // Diagonal 1,0 to 1,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 1u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}},
                        {{layer::DOWN, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}},
                        {{layer::RIGHT, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 1u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 1u}, {layer::DIAGONAL, 1u, 2u}},
                        {{layer::DOWN, 1u, 2u}, {layer::DIAGONAL, 1u, 2u}},
                        {{layer::RIGHT, 1u, 2u}, {layer::DIAGONAL, 1u, 2u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 1u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 2u}, {layer::DIAGONAL, 1u, 3u}},
                        {{layer::DOWN, 1u, 3u}, {layer::DIAGONAL, 1u, 3u}},
                        {{layer::RIGHT, 1u, 3u}, {layer::DIAGONAL, 1u, 3u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 1u, 3u})
            );
            // Diagonal 2,0 to 2,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DOWN, 2u, 0u}, {layer::DIAGONAL, 2u, 0u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 2u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 0u}, {layer::DIAGONAL, 2u, 1u}},
                        {{layer::DOWN, 2u, 1u}, {layer::DIAGONAL, 2u, 1u}},
                        {{layer::RIGHT, 2u, 1u}, {layer::DIAGONAL, 2u, 1u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 2u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 1u}, {layer::DIAGONAL, 2u, 2u}},
                        {{layer::DOWN, 2u, 2u}, {layer::DIAGONAL, 2u, 2u}},
                        {{layer::RIGHT, 2u, 2u}, {layer::DIAGONAL, 2u, 2u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 2u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 2u}, {layer::DIAGONAL, 2u, 3u}},
                        {{layer::DOWN, 2u, 3u}, {layer::DIAGONAL, 2u, 3u}},
                        {{layer::RIGHT, 2u, 3u}, {layer::DIAGONAL, 2u, 3u}}
                    }
                ),
                to_inputs({layer::DIAGONAL, 2u, 3u})
            );
            // Down 1,0 to 1,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}
                    }
                ),
                to_inputs({layer::DOWN, 1u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 1u}, {layer::DOWN, 1u, 1u}}
                    }
                ),
                to_inputs({layer::DOWN, 1u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 2u}, {layer::DOWN, 1u, 2u}}
                    }
                ),
                to_inputs({layer::DOWN, 1u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 3u}, {layer::DOWN, 1u, 3u}}
                    }
                ),
                to_inputs({layer::DOWN, 1u, 3u})
            );
            // Down 2,0 to 2,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 0u}, {layer::DOWN, 2u, 0u}},
                        {{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}
                    }
                ),
                to_inputs({layer::DOWN, 2u, 0u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 1u}, {layer::DOWN, 2u, 1u}},
                        {{layer::DOWN, 1u, 1u}, {layer::DOWN, 2u, 1u}}
                    }
                ),
                to_inputs({layer::DOWN, 2u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 2u}, {layer::DOWN, 2u, 2u}},
                        {{layer::DOWN, 1u, 2u}, {layer::DOWN, 2u, 2u}}
                    }
                ),
                to_inputs({layer::DOWN, 2u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 3u}, {layer::DOWN, 2u, 3u}},
                        {{layer::DOWN, 1u, 3u}, {layer::DOWN, 2u, 3u}}
                    }
                ),
                to_inputs({layer::DOWN, 2u, 3u})
            );
            // Right 0,1 to 0,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}},
                    }
                ),
                to_inputs({layer::RIGHT, 0u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 1u}, {layer::RIGHT, 0u, 2u}},
                        {{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}},
                    }
                ),
                to_inputs({layer::RIGHT, 0u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 0u, 2u}, {layer::RIGHT, 0u, 3u}},
                        {{layer::RIGHT, 0u, 2u}, {layer::RIGHT, 0u, 3u}},
                    }
                ),
                to_inputs({layer::RIGHT, 0u, 3u})
            );
            // Right 1,1 to 1,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 0u}, {layer::RIGHT, 1u, 1u}},
                    }
                ),
                to_inputs({layer::RIGHT, 1u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 1u}, {layer::RIGHT, 1u, 2u}},
                        {{layer::RIGHT, 1u, 1u}, {layer::RIGHT, 1u, 2u}},
                    }
                ),
                to_inputs({layer::RIGHT, 1u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 1u, 2u}, {layer::RIGHT, 1u, 3u}},
                        {{layer::RIGHT, 1u, 2u}, {layer::RIGHT, 1u, 3u}},
                    }
                ),
                to_inputs({layer::RIGHT, 1u, 3u})
            );
            // Right 2,1 to 2,3
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 2u, 0u}, {layer::RIGHT, 2u, 1u}},
                    }
                ),
                to_inputs({layer::RIGHT, 2u, 1u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 2u, 1u}, {layer::RIGHT, 2u, 2u}},
                        {{layer::RIGHT, 2u, 1u}, {layer::RIGHT, 2u, 2u}},
                    }
                ),
                to_inputs({layer::RIGHT, 2u, 2u})
            );
            EXPECT_EQ(
                (
                    std::set<E> {
                        {{layer::DIAGONAL, 2u, 2u}, {layer::RIGHT, 2u, 3u}},
                        {{layer::RIGHT, 2u, 2u}, {layer::RIGHT, 2u, 3u}},
                    }
                ),
                to_inputs({layer::RIGHT, 2u, 3u})
            );
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputDegree) {
        auto x = [](auto&& g) {
            // Diagonal 0,0 to 0,3
            EXPECT_EQ(3u, g.get_out_degree({layer::DIAGONAL, 0u, 0u}));
            EXPECT_EQ(3u, g.get_out_degree({layer::DIAGONAL, 0u, 1u}));
            EXPECT_EQ(3u, g.get_out_degree({layer::DIAGONAL, 0u, 2u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DIAGONAL, 0u, 3u}));
            // Diagonal 1,0 to 1,3
            EXPECT_EQ(3u, g.get_out_degree({layer::DIAGONAL, 1u, 0u}));
            EXPECT_EQ(3u, g.get_out_degree({layer::DIAGONAL, 1u, 1u}));
            EXPECT_EQ(3u, g.get_out_degree({layer::DIAGONAL, 1u, 2u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DIAGONAL, 1u, 3u}));
            // Diagonal 2,0 to 2,3
            EXPECT_EQ(1u, g.get_out_degree({layer::DIAGONAL, 2u, 0u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DIAGONAL, 2u, 1u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DIAGONAL, 2u, 2u}));
            EXPECT_EQ(0u, g.get_out_degree({layer::DIAGONAL, 2u, 3u}));
            // Down 1,0 to 1,3
            EXPECT_EQ(2u, g.get_out_degree({layer::DOWN, 1u, 0u}));
            EXPECT_EQ(2u, g.get_out_degree({layer::DOWN, 1u, 1u}));
            EXPECT_EQ(2u, g.get_out_degree({layer::DOWN, 1u, 2u}));
            EXPECT_EQ(2u, g.get_out_degree({layer::DOWN, 1u, 3u}));
            // Down 2,0 to 2,3
            EXPECT_EQ(1u, g.get_out_degree({layer::DOWN, 2u, 0u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DOWN, 2u, 1u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DOWN, 2u, 2u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::DOWN, 2u, 3u}));
            // Right 0,1 to 0,3
            EXPECT_EQ(2u, g.get_out_degree({layer::RIGHT, 0u, 1u}));
            EXPECT_EQ(2u, g.get_out_degree({layer::RIGHT, 0u, 2u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::RIGHT, 0u, 3u}));
            // Right 1,1 to 1,3
            EXPECT_EQ(2u, g.get_out_degree({layer::RIGHT, 1u, 1u}));
            EXPECT_EQ(2u, g.get_out_degree({layer::RIGHT, 1u, 2u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::RIGHT, 1u, 3u}));
            // Right 2,1 to 2,3
            EXPECT_EQ(2u, g.get_out_degree({layer::RIGHT, 2u, 1u}));
            EXPECT_EQ(2u, g.get_out_degree({layer::RIGHT, 2u, 2u}));
            EXPECT_EQ(1u, g.get_out_degree({layer::RIGHT, 2u, 3u}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputDegree) {
        auto x = [](auto&& g) {
            // Diagonal 0,0 to 0,3
            EXPECT_EQ(0u, g.get_in_degree({layer::DIAGONAL, 0u, 0u}));
            EXPECT_EQ(1u, g.get_in_degree({layer::DIAGONAL, 0u, 1u}));
            EXPECT_EQ(1u, g.get_in_degree({layer::DIAGONAL, 0u, 2u}));
            EXPECT_EQ(1u, g.get_in_degree({layer::DIAGONAL, 0u, 3u}));
            // Diagonal 1,0 to 1,3
            EXPECT_EQ(1u, g.get_in_degree({layer::DIAGONAL, 1u, 0u}));
            EXPECT_EQ(3u, g.get_in_degree({layer::DIAGONAL, 1u, 1u}));
            EXPECT_EQ(3u, g.get_in_degree({layer::DIAGONAL, 1u, 2u}));
            EXPECT_EQ(3u, g.get_in_degree({layer::DIAGONAL, 1u, 3u}));
            // Diagonal 2,0 to 2,3
            EXPECT_EQ(1u, g.get_in_degree({layer::DIAGONAL, 2u, 0u}));
            EXPECT_EQ(3u, g.get_in_degree({layer::DIAGONAL, 2u, 1u}));
            EXPECT_EQ(3u, g.get_in_degree({layer::DIAGONAL, 2u, 2u}));
            EXPECT_EQ(3u, g.get_in_degree({layer::DIAGONAL, 2u, 3u}));
            // Down 1,0 to 1,3
            EXPECT_EQ(1u, g.get_in_degree({layer::DOWN, 1u, 0u}));
            EXPECT_EQ(1u, g.get_in_degree({layer::DOWN, 1u, 1u}));
            EXPECT_EQ(1u, g.get_in_degree({layer::DOWN, 1u, 2u}));
            EXPECT_EQ(1u, g.get_in_degree({layer::DOWN, 1u, 3u}));
            // Down 2,0 to 2,3
            EXPECT_EQ(2u, g.get_in_degree({layer::DOWN, 2u, 0u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::DOWN, 2u, 1u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::DOWN, 2u, 2u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::DOWN, 2u, 3u}));
            // Right 0,1 to 0,3
            EXPECT_EQ(1u, g.get_in_degree({layer::RIGHT, 0u, 1u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::RIGHT, 0u, 2u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::RIGHT, 0u, 3u}));
            // Right 1,1 to 1,3
            EXPECT_EQ(1u, g.get_in_degree({layer::RIGHT, 1u, 1u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::RIGHT, 1u, 2u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::RIGHT, 1u, 3u}));
            // Right 2,1 to 2,3
            EXPECT_EQ(1u, g.get_in_degree({layer::RIGHT, 2u, 1u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::RIGHT, 2u, 2u}));
            EXPECT_EQ(2u, g.get_in_degree({layer::RIGHT, 2u, 3u}));
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetSetEdgeData) {
        auto x = [](auto&& g) {
            // Match updated
            g.update_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}, 1.0f);
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match (the one changed)
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 1u}, {layer::DIAGONAL, 1u, 2u}}));  // match (unchanged one)
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap
            g.update_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}, 0.0f);
            // Match to down indel updated
            g.update_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}, 1.0f);
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap (the one changed)
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from match to extended gap (should get changed as well)
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap (should get changed as well)
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap
            g.update_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}, 0.0f);
            // Match to right indel updated
            g.update_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}, 1.0f);
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap (should get changed as well)
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap (the one changed)
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from match to extended gap (should get changed as well)
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap
            g.update_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}, 0.0f);
            // Down extended gap indel updated
            g.update_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}, 1.0f);
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap (the one changed)
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::DOWN, 1u, 1u}, {layer::DOWN, 2u, 1u}}));  // from extended gap to extended gap (should get changed as well)
            EXPECT_EQ(1.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap (should get changed as well)
            g.update_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}, 0.0f);
            // Right extended gap indel updated
            g.update_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}, 2.0f);
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match
            EXPECT_EQ(2.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap (should get changed as well)
            EXPECT_EQ(2.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap (the one changed)
            EXPECT_EQ(2.0f, g.get_edge_data({{layer::RIGHT, 1u, 1u}, {layer::RIGHT, 1u, 2u}}));  // from extended gap to extended gap (should get changed as well)
            g.update_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}, 0.0f);
            // Right freeride back to match update
            g.update_edge_data({{layer::RIGHT, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}, 3.0f);
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap
            EXPECT_EQ(3.0f, g.get_edge_data({{layer::RIGHT, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}));  // from extended gap to match (the one changed)
            EXPECT_EQ(3.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match (should get changed as well)
            EXPECT_EQ(3.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match (should get changed as well)
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap
            g.update_edge_data({{layer::RIGHT, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}, 0.0f);
            // Right freeride back to match update
            g.update_edge_data({{layer::DOWN, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}, 4.0f);
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DIAGONAL, 1u, 1u}}));  // match
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::DOWN, 1u, 0u}}));  // from match to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DIAGONAL, 0u, 0u}, {layer::RIGHT, 0u, 1u}}));  // from match to extended gap
            EXPECT_EQ(4.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::DIAGONAL, 0u, 1u}}));  // from extended gap to match (should get changed as well)
            EXPECT_EQ(4.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DIAGONAL, 1u, 0u}}));  // from extended gap to match (should get changed as well)
            EXPECT_EQ(4.0f, g.get_edge_data({{layer::DOWN, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}));  // from extended gap to match (the one changed)
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::DOWN, 1u, 0u}, {layer::DOWN, 2u, 0u}}));  // from extended gap to extended gap
            EXPECT_EQ(0.0f, g.get_edge_data({{layer::RIGHT, 0u, 1u}, {layer::RIGHT, 0u, 2u}}));  // from extended gap to extended gap
            g.update_edge_data({{layer::DOWN, 1u, 1u}, {layer::DIAGONAL, 1u, 1u}}, 0.0f);
        };
        x(create_vector<float>(3u, 4u));
        x(create_array<float, 3u, 4u>());
        x(create_small_vector<float, 3u, 4u>(3u, 4u));
        x(create_static_vector<float, 3u, 4u>(3u, 4u));
    }
}