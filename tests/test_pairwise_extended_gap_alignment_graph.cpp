#include "pairwise_extended_gap_alignment_graph.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace offbynull::pairwise_aligner::extended_gap;

namespace {
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
                    N {layer::DIAGONAL, 1u, 0u},
                    N {layer::DIAGONAL, 1u, 1u},
                    N {layer::DIAGONAL, 1u, 2u},
                    N {layer::DOWN, 1u, 0u},
                    N {layer::DOWN, 1u, 1u},
                    N {layer::DOWN, 1u, 2u},
                    N {layer::RIGHT, 0u, 1u},
                    N {layer::RIGHT, 0u, 2u},
                    N {layer::RIGHT, 1u, 1u},
                    N {layer::RIGHT, 1u, 2u}
                })
            );
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
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
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N {layer::DIAGONAL, 0u, 0u}, N {layer::RIGHT, 0u, 1u} },
                    E { N {layer::DIAGONAL, 0u, 1u}, N {layer::RIGHT, 0u, 2u} },
                    E { N {layer::DIAGONAL, 1u, 0u}, N {layer::RIGHT, 1u, 1u} },
                    E { N {layer::DIAGONAL, 1u, 1u}, N {layer::RIGHT, 1u, 2u} },
                    E { N {layer::DIAGONAL, 0u, 0u}, N {layer::DOWN, 1u, 0u} },
                    E { N {layer::DIAGONAL, 0u, 1u}, N {layer::DOWN, 1u, 1u} },
                    E { N {layer::DIAGONAL, 0u, 2u}, N {layer::DOWN, 1u, 2u} },
                    E { N {layer::DIAGONAL, 0u, 0u}, N {layer::DIAGONAL, 1u, 1u} },
                    E { N {layer::DIAGONAL, 0u, 1u}, N {layer::DIAGONAL, 1u, 2u} },
                    E { N {layer::RIGHT, 0u, 1u}, N {layer::DIAGONAL, 0u, 1u} },
                    E { N {layer::RIGHT, 0u, 2u}, N {layer::DIAGONAL, 0u, 2u} },
                    E { N {layer::RIGHT, 1u, 1u}, N {layer::DIAGONAL, 1u, 1u} },
                    E { N {layer::RIGHT, 1u, 2u}, N {layer::DIAGONAL, 1u, 2u} },
                    E { N {layer::DOWN, 1u, 0u}, N {layer::DIAGONAL, 1u, 0u} },
                    E { N {layer::DOWN, 1u, 1u}, N {layer::DIAGONAL, 1u, 1u} },
                    E { N {layer::DOWN, 1u, 2u}, N {layer::DIAGONAL, 1u, 2u} },
                })
            );
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    CONTINUE TESTING;
    CONTINUE TESTING;
    CONTINUE TESTING;
    CONTINUE TESTING;
    CONTINUE TESTING;
    /*
    TEST(PairwiseExtendedGapAlignmentGraphTest, NodesExist) {
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
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {0u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {0u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {0u, 3u}}));
            EXPECT_TRUE(g.has_edge({{1u, 0u}, {1u, 1u}}));
            EXPECT_TRUE(g.has_edge({{1u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {1u, 3u}}));
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {1u, 0u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 0u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 2u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 2u}}));
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_TRUE(g.has_edge({{0u, 0u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 1u}}));
            EXPECT_TRUE(g.has_edge({{0u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {1u, 3u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 3u}}));
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputs) {
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
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputs) {
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
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 0u } ), 3);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 2u } ), 0);
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 0u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 2u } ), 3);
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector_grid<float>(2u, 3u));
        x(create_array_grid<float, 2u, 3u>());
        x(create_small_vector_grid<float, 2u, 3u>(2u, 3u));
        x(create_static_vector_grid<float, 2u, 3u>(2u, 3u));
    }
*/
}