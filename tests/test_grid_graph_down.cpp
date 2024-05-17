 #include "grid_graph_down.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace offbynull::grid_graph::grid_graph_down;

namespace {
    TEST(GridGraphRightTest, ListNodes) {
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

    TEST(GridGraphRightTest, ListEdges) {
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
                    std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} },
                    std::pair { std::pair{0u, 1u}, std::pair{1u, 1u} },
                    std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} }
                })
            );
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphRightTest, NodesExist) {
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

    TEST(GridGraphRightTest, RightEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_FALSE(g.has_edge({{0u, 0u}, {0u, 1u}}));
            EXPECT_FALSE(g.has_edge({{0u, 1u}, {0u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {0u, 3u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {1u, 3u}}));
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphRightTest, DownEdgesExist) {
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

    TEST(GridGraphRightTest, DiagEdgesExist) {
        auto x = [](auto&& g) {
            EXPECT_FALSE(g.has_edge({{0u, 0u}, {1u, 1u}}));
            EXPECT_FALSE(g.has_edge({{1u, 0u}, {2u, 1u}}));
            EXPECT_FALSE(g.has_edge({{0u, 1u}, {1u, 2u}}));
            EXPECT_FALSE(g.has_edge({{1u, 1u}, {2u, 2u}}));
            EXPECT_FALSE(g.has_edge({{0u, 2u}, {1u, 3u}}));
            EXPECT_FALSE(g.has_edge({{1u, 2u}, {2u, 3u}}));
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphRightTest, GetOutputs) {
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
                        std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} }
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
                    (std::set<E> {})
                );
            }
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphRightTest, GetInputs) {
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
                        std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} }
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
                    (std::set<E> {})
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

    TEST(GridGraphRightTest, GetOutputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 0u } ), 1);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 2u } ), 0);
            EXPECT_EQ(g.get_out_degree(std::pair{ 0u, 2u } ), 1);
            EXPECT_EQ(g.get_out_degree(std::pair{ 1u, 0u } ), 0);
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }

    TEST(GridGraphRightTest, GetInputDegree) {
        auto x = [](auto&& g) {
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 0u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 2u } ), 1);
            EXPECT_EQ(g.get_in_degree(std::pair{ 0u, 2u } ), 0);
            EXPECT_EQ(g.get_in_degree(std::pair{ 1u, 0u } ), 1);
        };
        x(create_vector<std::string, std::string>(2u, 3u));
        x(create_array<std::string, std::string, 2u, 3u>());
        x(create_small_vector<std::string, std::string, 2u, 3u>(2u, 3u));
        x(create_static_vector<std::string, std::string, 2u, 3u>(2u, 3u));
    }
}