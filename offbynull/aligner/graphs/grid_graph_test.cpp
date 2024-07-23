#include <cstddef>
#include <limits>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto substitution_scorer { simple_scorer<char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<char, char, std::float64_t>::create_gap(0.0f64) };

    TEST(GridGraphTest, ConceptCheck) {
        using G = grid_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
    }

    TEST(GridGraphTest, ListNodes) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                std::pair{0zu, 0zu}, std::pair{0zu, 1zu}, std::pair{0zu, 2zu},
                std::pair{1zu, 0zu}, std::pair{1zu, 1zu}, std::pair{1zu, 2zu}
            })
        );
    }

    TEST(GridGraphTest, ListEdges) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        auto e = g.get_edges();
        std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
        for (auto _e : e) {
            actual.insert(_e);
        }
        EXPECT_EQ(
            actual,
            (std::set<E> {
                std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} },
                std::pair { std::pair{0zu, 1zu}, std::pair{0zu, 2zu} },
                std::pair { std::pair{1zu, 0zu}, std::pair{1zu, 1zu} },
                std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} },
                std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} },
                std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 1zu} },
                std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} },
                std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} },
                std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 2zu} }
            })
        );
    }

    TEST(GridGraphTest, NodesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_node({0zu, 0zu}));
        EXPECT_TRUE(g.has_node({0zu, 1zu}));
        EXPECT_TRUE(g.has_node({0zu, 2zu}));
        EXPECT_FALSE(g.has_node({0zu, 3zu}));
        EXPECT_TRUE(g.has_node({1zu, 0zu}));
        EXPECT_TRUE(g.has_node({1zu, 1zu}));
        EXPECT_TRUE(g.has_node({1zu, 2zu}));
        EXPECT_FALSE(g.has_node({1zu, 3zu}));
        EXPECT_FALSE(g.has_node({2zu, 3zu}));
    }

    TEST(GridGraphTest, RightEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({{0zu, 0zu}, {0zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{0zu, 1zu}, {0zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{0zu, 2zu}, {0zu, 3zu}}));
        EXPECT_TRUE(g.has_edge({{1zu, 0zu}, {1zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{1zu, 1zu}, {1zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 2zu}, {1zu, 3zu}}));
    }

    TEST(GridGraphTest, DownEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({{0zu, 0zu}, {1zu, 0zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 0zu}, {2zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{0zu, 1zu}, {1zu, 1zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 1zu}, {2zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{0zu, 2zu}, {1zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 2zu}, {2zu, 2zu}}));
    }

    TEST(GridGraphTest, DiagEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({{0zu, 0zu}, {1zu, 1zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 0zu}, {2zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{0zu, 1zu}, {1zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 1zu}, {2zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{0zu, 2zu}, {1zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{1zu, 2zu}, {2zu, 3zu}}));
    }

    TEST(GridGraphTest, GetOutputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( std::pair{ 0zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} },
                    std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} },
                    std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( std::pair{ 1zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {})
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( std::pair{ 0zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( std::pair{ 1zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{1zu, 0zu}, std::pair{1zu, 1zu} }
                })
            );
        }
    }

    TEST(GridGraphTest, GetInputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( std::pair{ 0zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {})
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( std::pair{ 1zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} },
                    std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} },
                    std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 2zu} }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( std::pair{0zu, 2zu} )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{0zu, 1zu}, std::pair{0zu, 2zu} }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( std::pair{1zu, 0zu} )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} }
                })
            );
        }
    }

    TEST(GridGraphTest, GetOutputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_EQ(g.get_out_degree(std::pair{ 0zu, 0zu } ), 3);
        EXPECT_EQ(g.get_out_degree(std::pair{ 1zu, 2zu } ), 0);
        EXPECT_EQ(g.get_out_degree(std::pair{ 0zu, 2zu } ), 1);
        EXPECT_EQ(g.get_out_degree(std::pair{ 1zu, 0zu } ), 1);
    }

    TEST(GridGraphTest, GetInputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        grid_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_EQ(g.get_in_degree(std::pair{ 0zu, 0zu } ), 0);
        EXPECT_EQ(g.get_in_degree(std::pair{ 1zu, 2zu } ), 3);
        EXPECT_EQ(g.get_in_degree(std::pair{ 0zu, 2zu } ), 1);
        EXPECT_EQ(g.get_in_degree(std::pair{ 1zu, 0zu } ), 1);
    }
}