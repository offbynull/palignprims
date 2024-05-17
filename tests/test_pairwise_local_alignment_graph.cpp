#include "pairwise_local_alignment_graph.h"
#include "gtest/gtest.h"
#include <iostream>
#include <map>

using namespace offbynull::pairwise_aligner::local;

namespace {
    TEST(PairwiseLocalAlignmentGraph, ListNodes) {
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

    TEST(PairwiseLocalAlignmentGraph, ListEdges) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseLocalAlignmentGraph, NodesExist) {
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

    TEST(PairwiseLocalAlignmentGraph, RightEdgesExist) {
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

    TEST(PairwiseLocalAlignmentGraph, DownEdgesExist) {
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

    TEST(PairwiseLocalAlignmentGraph, DiagEdgesExist) {
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

    TEST(PairwiseLocalAlignmentGraph, FreeRideEdgesExist) {
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
        x(create_vector<float>(2u, 3u));
        x(create_array<float, 2u, 3u>());
        x(create_small_vector<float, 2u, 3u>(2u, 3u));
        x(create_static_vector<float, 2u, 3u>(2u, 3u));
    }

    TEST(PairwiseLocalAlignmentGraph, GetOutputs) {
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

    TEST(PairwiseLocalAlignmentGraph, GetInputs) {
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

    TEST(PairwiseLocalAlignmentGraph, GetOutputDegree) {
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

    TEST(PairwiseLocalAlignmentGraph, GetInputDegree) {
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