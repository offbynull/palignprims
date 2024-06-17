#include <cstddef>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::slot;

    template<typename ED, typename INDEX = unsigned int, bool error_check = true>
    auto create_vector(INDEX down_cnt, INDEX right_cnt) {
        return pairwise_extended_gap_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<slot<std::tuple<>,ED>, INDEX>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ED, std::size_t STATIC_DOWN_CNT, std::size_t STATIC_RIGHT_CNT, typename INDEX = unsigned int, bool error_check = true>
    auto create_array() {
        return pairwise_extended_gap_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::array_grid_container_creator<slot<std::tuple<>, ED>, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT
        };
    }

    template<typename ED, std::size_t STATIC_DOWN_CNT, std::size_t STATIC_RIGHT_CNT, typename INDEX = unsigned int, bool error_check = true>
    auto create_small_vector(INDEX down_cnt, INDEX right_cnt) {
        return pairwise_extended_gap_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::static_vector_grid_container_creator<slot<std::tuple<>, ED>, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    template<typename ED, std::size_t STATIC_DOWN_CNT, std::size_t STATIC_RIGHT_CNT, typename INDEX = unsigned int, bool error_check = true>
    auto create_static_vector(INDEX down_cnt, INDEX right_cnt) {
        return pairwise_extended_gap_alignment_graph<
            std::tuple<>,
            ED,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::small_vector_grid_container_creator<slot<std::tuple<>, ED>, INDEX, STATIC_DOWN_CNT, STATIC_RIGHT_CNT>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_extended_gap_alignment_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph<G>);
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
                    std::set<E> ret {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                    for (auto e : g.get_outputs(n)) {
                        ret.insert(e);
                    }
                    return ret;
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
                    std::set<E> ret {}; // TODO: I can't use being() and end() within set's constructor to automate this?
                    for (auto e : g.get_inputs(n)) {
                        ret.insert(e);
                    }
                    return ret;
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

    TEST(PairwiseExtendedGapAlignmentGraphTest, SlicedWalk) {
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

            EXPECT_EQ(G::slice_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 7zu);
            EXPECT_EQ(g.slice_first_node(0u), (N { layer::DIAGONAL, 0u, 0u }));
            EXPECT_EQ(g.slice_last_node(0u), (N { layer::DOWN, 0u, 2u }));
            EXPECT_EQ(g.slice_first_node(1u), (N { layer::DIAGONAL, 1u, 0u }));
            EXPECT_EQ(g.slice_last_node(1u), (N { layer::DOWN, 1u, 2u }));

            EXPECT_EQ(G::resident_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 0zu);
            EXPECT_EQ(g.resident_nodes().size(), 0zu);

            EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 0u, 0u }), (N { layer::DOWN, 0u, 1u })); // n_down=0
            EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 0u, 1u }), (N { layer::RIGHT, 0u, 1u }));
            EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 0u, 1u }), (N { layer::DIAGONAL, 0u, 1u }));
            EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 0u, 1u }), (N { layer::DOWN, 0u, 2u }));
            EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 0u, 2u }), (N { layer::RIGHT, 0u, 2u }));
            EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 0u, 2u }), (N { layer::DIAGONAL, 0u, 2u }));
            EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 1u, 0u }), (N { layer::DOWN, 1u, 1u })); // n_down=1
            EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 1u, 1u }), (N { layer::RIGHT, 1u, 1u }));
            EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 1u, 1u }), (N { layer::DIAGONAL, 1u, 1u }));
            EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 1u, 1u }), (N { layer::DOWN, 1u, 2u }));
            EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 1u, 2u }), (N { layer::RIGHT, 1u, 2u }));
            EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 1u, 2u }), (N { layer::DIAGONAL, 1u, 2u }));

            EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 1u, 2u }), (N { layer::RIGHT, 1u, 2u })); // n_down=1
            EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 1u, 2u }), (N { layer::DOWN, 1u, 2u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 1u, 2u }), (N { layer::DIAGONAL, 1u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 1u, 1u }), (N { layer::RIGHT, 1u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 1u, 1u }), (N { layer::DOWN, 1u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 1u, 1u }), (N { layer::DIAGONAL, 1u, 0u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 0u, 2u }), (N { layer::RIGHT, 0u, 2u })); // n_down=1
            EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 0u, 2u }), (N { layer::DOWN, 0u, 2u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 0u, 2u }), (N { layer::DIAGONAL, 0u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 0u, 1u }), (N { layer::RIGHT, 0u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 0u, 1u }), (N { layer::DOWN, 0u, 1u }));
            EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 0u, 1u }), (N { layer::DIAGONAL, 0u, 0u }));

            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 0u, 0u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 0u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 0u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 0u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 0u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 0u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 1u, 0u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 1u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 1u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 1u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 1u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 1u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 1u, 2u })), (std::vector<E> {}));

            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 0u, 0u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 0u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 0u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 0u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 0u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 0u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 1u, 0u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 1u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 1u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 1u, 1u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 1u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 1u, 2u })), (std::vector<E> {}));
            EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 1u, 2u })), (std::vector<E> {}));
        };
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }
}