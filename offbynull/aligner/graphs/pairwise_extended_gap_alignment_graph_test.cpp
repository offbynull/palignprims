#include <cstddef>
#include <algorithm>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node_layer;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
    auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.1f64) };
    auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride() };

    TEST(PairwiseExtendedGapAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_extended_gap_alignment_graph<true, std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ListNodes) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        std::set<N> actual {};
        for (const auto &n : g.get_nodes()) {
            actual.insert(n);
        }
        EXPECT_EQ(
            actual,
            (std::set<N> {
                N { node_layer::DIAGONAL, 0zu, 0zu },
                N { node_layer::DIAGONAL, 0zu, 1zu },
                N { node_layer::DIAGONAL, 0zu, 2zu },
                N { node_layer::DIAGONAL, 0zu, 3zu },
                N { node_layer::DIAGONAL, 1zu, 0zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 3zu },
                N { node_layer::DIAGONAL, 2zu, 0zu },
                N { node_layer::DIAGONAL, 2zu, 1zu },
                N { node_layer::DIAGONAL, 2zu, 2zu },
                N { node_layer::DIAGONAL, 2zu, 3zu },
                N { node_layer::DOWN, 1zu, 0zu },
                N { node_layer::DOWN, 1zu, 1zu },
                N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::DOWN, 1zu, 3zu },
                N { node_layer::DOWN, 2zu, 0zu },
                N { node_layer::DOWN, 2zu, 1zu },
                N { node_layer::DOWN, 2zu, 2zu },
                N { node_layer::DOWN, 2zu, 3zu },
                N { node_layer::RIGHT, 0zu, 1zu },
                N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::RIGHT, 2zu, 1zu },
                N { node_layer::RIGHT, 0zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::RIGHT, 2zu, 2zu },
                N { node_layer::RIGHT, 0zu, 3zu },
                N { node_layer::RIGHT, 1zu, 3zu },
                N { node_layer::RIGHT, 2zu, 3zu }
            })
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ListEdges) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        auto e = g.get_edges();
        std::vector<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
        for (auto _e : e) {
            actual.push_back(_e);
        }
        std::ranges::sort(actual);
        std::vector<E> expected {
            E { N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::DIAGONAL, 1zu, 1zu } },
            E { N { node_layer::DIAGONAL, 0zu, 1zu }, N { node_layer::DIAGONAL, 1zu, 2zu } },
            E { N { node_layer::DIAGONAL, 0zu, 2zu }, N { node_layer::DIAGONAL, 1zu, 3zu } },
            E { N { node_layer::DIAGONAL, 1zu, 0zu }, N { node_layer::DIAGONAL, 2zu, 1zu } },
            E { N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::DIAGONAL, 2zu, 2zu } },
            E { N { node_layer::DIAGONAL, 1zu, 2zu }, N { node_layer::DIAGONAL, 2zu, 3zu } },
            E { N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::RIGHT, 0zu, 1zu } },
            E { N { node_layer::DIAGONAL, 0zu, 1zu }, N { node_layer::RIGHT, 0zu, 2zu } },
            E { N { node_layer::DIAGONAL, 0zu, 2zu }, N { node_layer::RIGHT, 0zu, 3zu } },
            E { N { node_layer::DIAGONAL, 1zu, 0zu }, N { node_layer::RIGHT, 1zu, 1zu } },
            E { N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::RIGHT, 1zu, 2zu } },
            E { N { node_layer::DIAGONAL, 1zu, 2zu }, N { node_layer::RIGHT, 1zu, 3zu } },
            E { N { node_layer::DIAGONAL, 2zu, 0zu }, N { node_layer::RIGHT, 2zu, 1zu } },
            E { N { node_layer::DIAGONAL, 2zu, 1zu }, N { node_layer::RIGHT, 2zu, 2zu } },
            E { N { node_layer::DIAGONAL, 2zu, 2zu }, N { node_layer::RIGHT, 2zu, 3zu } },
            E { N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::DOWN, 1zu, 0zu } },
            E { N { node_layer::DIAGONAL, 1zu, 0zu }, N { node_layer::DOWN, 2zu, 0zu } },
            E { N { node_layer::DIAGONAL, 0zu, 1zu }, N { node_layer::DOWN, 1zu, 1zu } },
            E { N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::DOWN, 2zu, 1zu } },
            E { N { node_layer::DIAGONAL, 0zu, 2zu }, N { node_layer::DOWN, 1zu, 2zu } },
            E { N { node_layer::DIAGONAL, 1zu, 2zu }, N { node_layer::DOWN, 2zu, 2zu } },
            E { N { node_layer::DIAGONAL, 0zu, 3zu }, N { node_layer::DOWN, 1zu, 3zu } },
            E { N { node_layer::DIAGONAL, 1zu, 3zu }, N { node_layer::DOWN, 2zu, 3zu } },
            E { N { node_layer::DOWN, 1zu, 0zu }, N { node_layer::DIAGONAL, 1zu, 0zu } },
            E { N { node_layer::DOWN, 1zu, 1zu }, N { node_layer::DIAGONAL, 1zu, 1zu } },
            E { N { node_layer::DOWN, 1zu, 2zu }, N { node_layer::DIAGONAL, 1zu, 2zu } },
            E { N { node_layer::DOWN, 1zu, 3zu }, N { node_layer::DIAGONAL, 1zu, 3zu } },
            E { N { node_layer::DOWN, 2zu, 0zu }, N { node_layer::DIAGONAL, 2zu, 0zu } },
            E { N { node_layer::DOWN, 2zu, 1zu }, N { node_layer::DIAGONAL, 2zu, 1zu } },
            E { N { node_layer::DOWN, 2zu, 2zu }, N { node_layer::DIAGONAL, 2zu, 2zu } },
            E { N { node_layer::DOWN, 2zu, 3zu }, N { node_layer::DIAGONAL, 2zu, 3zu } },
            E { N { node_layer::DOWN, 1zu, 0zu }, N { node_layer::DOWN, 2zu, 0zu } },
            E { N { node_layer::DOWN, 1zu, 1zu }, N { node_layer::DOWN, 2zu, 1zu } },
            E { N { node_layer::DOWN, 1zu, 2zu }, N { node_layer::DOWN, 2zu, 2zu } },
            E { N { node_layer::DOWN, 1zu, 3zu }, N { node_layer::DOWN, 2zu, 3zu } },
            E { N { node_layer::RIGHT, 0zu, 1zu }, N { node_layer::DIAGONAL, 0zu, 1zu } },
            E { N { node_layer::RIGHT, 0zu, 2zu }, N { node_layer::DIAGONAL, 0zu, 2zu } },
            E { N { node_layer::RIGHT, 0zu, 3zu }, N { node_layer::DIAGONAL, 0zu, 3zu } },
            E { N { node_layer::RIGHT, 1zu, 1zu }, N { node_layer::DIAGONAL, 1zu, 1zu } },
            E { N { node_layer::RIGHT, 1zu, 2zu }, N { node_layer::DIAGONAL, 1zu, 2zu } },
            E { N { node_layer::RIGHT, 1zu, 3zu }, N { node_layer::DIAGONAL, 1zu, 3zu } },
            E { N { node_layer::RIGHT, 2zu, 1zu }, N { node_layer::DIAGONAL, 2zu, 1zu } },
            E { N { node_layer::RIGHT, 2zu, 2zu }, N { node_layer::DIAGONAL, 2zu, 2zu } },
            E { N { node_layer::RIGHT, 2zu, 3zu }, N { node_layer::DIAGONAL, 2zu, 3zu } },
            E { N { node_layer::RIGHT, 0zu, 1zu }, N { node_layer::RIGHT, 0zu, 2zu } },
            E { N { node_layer::RIGHT, 0zu, 2zu }, N { node_layer::RIGHT, 0zu, 3zu } },
            E { N { node_layer::RIGHT, 1zu, 1zu }, N { node_layer::RIGHT, 1zu, 2zu } },
            E { N { node_layer::RIGHT, 1zu, 2zu }, N { node_layer::RIGHT, 1zu, 3zu } },
            E { N { node_layer::RIGHT, 2zu, 1zu }, N { node_layer::RIGHT, 2zu, 2zu } },
            E { N { node_layer::RIGHT, 2zu, 2zu }, N { node_layer::RIGHT, 2zu, 3zu } }
        };
        std::ranges::sort(expected);
        EXPECT_EQ(
            actual,
            expected
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, NodesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 0zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 0zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 0zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 0zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DIAGONAL, 0zu, 4zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 1zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 1zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 1zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 1zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DIAGONAL, 1zu, 4zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 2zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 2zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 2zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DIAGONAL, 2zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DIAGONAL, 2zu, 4zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DIAGONAL, 3zu, 3zu }));

        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 0zu, 0zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 0zu, 1zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 0zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 0zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 0zu, 4zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 1zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 1zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 1zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 1zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 1zu, 4zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 2zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 2zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 2zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::DOWN, 2zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 2zu, 4zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::DOWN, 3zu, 3zu }));

        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 0zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 0zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 0zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 0zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 0zu, 4zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 1zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 1zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 1zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 1zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 1zu, 4zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 2zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 2zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 2zu, 2zu }));
        EXPECT_TRUE(g.has_node(N { node_layer::RIGHT, 2zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 2zu, 4zu }));
        EXPECT_FALSE(g.has_node(N { node_layer::RIGHT, 3zu, 3zu }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::RIGHT, 0zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::RIGHT, 0zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::RIGHT, 0zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 3zu }, { node_layer::RIGHT, 0zu, 4zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::RIGHT, 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::RIGHT, 1zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::RIGHT, 1zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 3zu }, { node_layer::RIGHT, 1zu, 4zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 0zu }, { node_layer::RIGHT, 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 1zu }, { node_layer::RIGHT, 2zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 2zu }, { node_layer::RIGHT, 2zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 3zu }, { node_layer::RIGHT, 2zu, 4zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 3zu, 0zu }, { node_layer::RIGHT, 3zu, 1zu } }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DOWN, 1zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::DOWN, 2zu, 0zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 0zu }, { node_layer::DOWN, 3zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DOWN, 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::DOWN, 2zu, 1zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 1zu }, { node_layer::DOWN, 3zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::DOWN, 1zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::DOWN, 2zu, 2zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 2zu }, { node_layer::DOWN, 3zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 3zu }, { node_layer::DOWN, 1zu, 3zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 3zu }, { node_layer::DOWN, 2zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 3zu }, { node_layer::DOWN, 3zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 3zu, 0zu }, { node_layer::DOWN, 4zu, 0zu } }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DiagEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DIAGONAL, 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DIAGONAL, 1zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::DIAGONAL, 1zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 0zu, 3zu }, { node_layer::DIAGONAL, 1zu, 4zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::DIAGONAL, 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::DIAGONAL, 2zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::DIAGONAL, 2zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 1zu, 3zu }, { node_layer::DIAGONAL, 2zu, 4zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DIAGONAL, 2zu, 0zu }, { node_layer::DIAGONAL, 3zu, 1zu } }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownFreeRidgeEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 0zu, 0zu }, { node_layer::DIAGONAL, 0zu, 0zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 0zu, 1zu }, { node_layer::DIAGONAL, 0zu, 1zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 0zu, 2zu }, { node_layer::DIAGONAL, 0zu, 2zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 0zu, 3zu }, { node_layer::DIAGONAL, 0zu, 3zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DIAGONAL, 1zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 1zu, 1zu }, { node_layer::DIAGONAL, 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 1zu, 2zu }, { node_layer::DIAGONAL, 1zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 1zu, 3zu }, { node_layer::DIAGONAL, 1zu, 3zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 2zu, 0zu }, { node_layer::DIAGONAL, 2zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 2zu, 1zu }, { node_layer::DIAGONAL, 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 2zu, 2zu }, { node_layer::DIAGONAL, 2zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::DOWN, 2zu, 3zu }, { node_layer::DIAGONAL, 2zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 3zu, 0zu }, { node_layer::DIAGONAL, 3zu, 0zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 3zu, 1zu }, { node_layer::DIAGONAL, 3zu, 1zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 3zu, 2zu }, { node_layer::DIAGONAL, 3zu, 2zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::DOWN, 3zu, 3zu }, { node_layer::DIAGONAL, 3zu, 3zu } }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightFreeRidgeEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 0zu, 0zu }, { node_layer::DIAGONAL, 0zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::DIAGONAL, 0zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 0zu, 2zu }, { node_layer::DIAGONAL, 0zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 0zu, 3zu }, { node_layer::DIAGONAL, 0zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 1zu, 0zu }, { node_layer::DIAGONAL, 1zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 1zu, 1zu }, { node_layer::DIAGONAL, 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 1zu, 2zu }, { node_layer::DIAGONAL, 1zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 1zu, 3zu }, { node_layer::DIAGONAL, 1zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 2zu, 0zu }, { node_layer::DIAGONAL, 2zu, 0zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 2zu, 1zu }, { node_layer::DIAGONAL, 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 2zu, 2zu }, { node_layer::DIAGONAL, 2zu, 2zu } }));
        EXPECT_TRUE(g.has_edge(E { { node_layer::RIGHT, 2zu, 3zu }, { node_layer::DIAGONAL, 2zu, 3zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 3zu, 0zu }, { node_layer::DIAGONAL, 3zu, 0zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 3zu, 1zu }, { node_layer::DIAGONAL, 3zu, 1zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 3zu, 2zu }, { node_layer::DIAGONAL, 3zu, 2zu } }));
        EXPECT_FALSE(g.has_edge(E { { node_layer::RIGHT, 3zu, 3zu }, { node_layer::DIAGONAL, 3zu, 3zu } }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputs) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

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
                    E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DIAGONAL, 1zu, 1zu } },
                    E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DOWN, 1zu, 0zu } },
                    E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::RIGHT, 0zu, 1zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 0zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DIAGONAL, 1zu, 2zu } },
                    E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DOWN, 1zu, 1zu } },
                    E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::RIGHT, 0zu, 2zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 0zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::DIAGONAL, 1zu, 3zu } },
                    E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::DOWN, 1zu, 2zu } },
                    E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::RIGHT, 0zu, 3zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 0zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 3zu }, { node_layer::DOWN, 1zu, 3zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 0zu, 3zu })
        );
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::DIAGONAL, 2zu, 1zu } },
                    E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::DOWN, 2zu, 0zu } },
                    E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::RIGHT, 1zu, 1zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 1zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::DIAGONAL, 2zu, 2zu } },
                    E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::DOWN, 2zu, 1zu } },
                    E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::RIGHT, 1zu, 2zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 1zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::DIAGONAL, 2zu, 3zu } },
                    E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::DOWN, 2zu, 2zu } },
                    E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::RIGHT, 1zu, 3zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 1zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 3zu }, { node_layer::DOWN, 2zu, 3zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 1zu, 3zu })
        );
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 2zu, 0zu }, { node_layer::RIGHT, 2zu, 1zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 2zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 2zu, 1zu }, { node_layer::RIGHT, 2zu, 2zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 2zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 2zu, 2zu }, { node_layer::RIGHT, 2zu, 3zu } }
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 2zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                }
            ),
            to_outputs(N { node_layer::DIAGONAL, 2zu, 3zu })
        );
        // Down 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DIAGONAL, 1zu, 0zu } },
                    E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DOWN, 2zu, 0zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 1zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 1zu, 1zu }, { node_layer::DIAGONAL, 1zu, 1zu } },
                    E { { node_layer::DOWN, 1zu, 1zu }, { node_layer::DOWN, 2zu, 1zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 1zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 1zu, 2zu }, { node_layer::DIAGONAL, 1zu, 2zu } },
                    E { { node_layer::DOWN, 1zu, 2zu }, { node_layer::DOWN, 2zu, 2zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 1zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 1zu, 3zu }, { node_layer::DIAGONAL, 1zu, 3zu } },
                    E { { node_layer::DOWN, 1zu, 3zu }, { node_layer::DOWN, 2zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 1zu, 3zu })
        );
        // Down 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 2zu, 0zu }, { node_layer::DIAGONAL, 2zu, 0zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 2zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 2zu, 1zu }, { node_layer::DIAGONAL, 2zu, 1zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 2zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 2zu, 2zu }, { node_layer::DIAGONAL, 2zu, 2zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 2zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 2zu, 3zu }, { node_layer::DIAGONAL, 2zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::DOWN, 2zu, 3zu })
        );
        // Right 0,1 to 0,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::DIAGONAL, 0zu, 1zu } },
                    E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::RIGHT, 0zu, 2zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 0zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 0zu, 2zu }, { node_layer::DIAGONAL , 0zu, 2zu } },
                    E { { node_layer::RIGHT, 0zu, 2zu }, { node_layer::RIGHT , 0zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 0zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 0zu, 3zu }, { node_layer::DIAGONAL, 0zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 0zu, 3zu })
        );
        // Right 1,1 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 1zu, 1zu }, { node_layer::DIAGONAL, 1zu, 1zu } },
                    E { { node_layer::RIGHT, 1zu, 1zu }, { node_layer::RIGHT, 1zu, 2zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 1zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 1zu, 2zu }, { node_layer::DIAGONAL, 1zu, 2zu } },
                    E { { node_layer::RIGHT, 1zu, 2zu }, { node_layer::RIGHT, 1zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 1zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 1zu, 3zu }, { node_layer::DIAGONAL, 1zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 1zu, 3zu })
        );
        // Right 2,1 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 2zu, 1zu }, { node_layer::DIAGONAL, 2zu, 1zu } },
                    E { { node_layer::RIGHT, 2zu, 1zu }, { node_layer::RIGHT, 2zu, 2zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 2zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 2zu, 2zu }, { node_layer::DIAGONAL, 2zu, 2zu } },
                    E { { node_layer::RIGHT, 2zu, 2zu }, { node_layer::RIGHT, 2zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 2zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 2zu, 3zu }, { node_layer::DIAGONAL, 2zu, 3zu } },
                }
            ),
            to_outputs(N { node_layer::RIGHT, 2zu, 3zu })
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputs) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

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
            to_inputs(N { node_layer::DIAGONAL, 0zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::DIAGONAL, 0zu, 1zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 0zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 0zu, 2zu }, { node_layer::DIAGONAL, 0zu, 2zu } },
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 0zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::RIGHT, 0zu, 3zu }, { node_layer::DIAGONAL, 0zu, 3zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 0zu, 3zu })
        );
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DIAGONAL, 1zu, 0zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 1zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DIAGONAL, 1zu, 1zu } },
                    E { { node_layer::DOWN, 1zu, 1zu }, { node_layer::DIAGONAL, 1zu, 1zu } },
                    E { { node_layer::RIGHT, 1zu, 1zu }, { node_layer::DIAGONAL, 1zu, 1zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 1zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DIAGONAL, 1zu, 2zu } },
                    E { { node_layer::DOWN, 1zu, 2zu }, { node_layer::DIAGONAL, 1zu, 2zu } },
                    E { { node_layer::RIGHT, 1zu, 2zu }, { node_layer::DIAGONAL, 1zu, 2zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 1zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::DIAGONAL, 1zu, 3zu } },
                    E { { node_layer::DOWN, 1zu, 3zu }, { node_layer::DIAGONAL, 1zu, 3zu } },
                    E { { node_layer::RIGHT, 1zu, 3zu }, { node_layer::DIAGONAL, 1zu, 3zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 1zu, 3zu })
        );
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DOWN, 2zu, 0zu }, { node_layer::DIAGONAL, 2zu, 0zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 2zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::DIAGONAL, 2zu, 1zu } },
                    E { { node_layer::DOWN, 2zu, 1zu }, { node_layer::DIAGONAL, 2zu, 1zu } },
                    E { { node_layer::RIGHT, 2zu, 1zu }, { node_layer::DIAGONAL, 2zu, 1zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 2zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::DIAGONAL, 2zu, 2zu } },
                    E { { node_layer::DOWN, 2zu, 2zu }, { node_layer::DIAGONAL, 2zu, 2zu } },
                    E { { node_layer::RIGHT, 2zu, 2zu }, { node_layer::DIAGONAL, 2zu, 2zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 2zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::DIAGONAL, 2zu, 3zu } },
                    E { { node_layer::DOWN, 2zu, 3zu }, { node_layer::DIAGONAL, 2zu, 3zu } },
                    E { { node_layer::RIGHT, 2zu, 3zu }, { node_layer::DIAGONAL, 2zu, 3zu } }
                }
            ),
            to_inputs(N { node_layer::DIAGONAL, 2zu, 3zu })
        );
        // Down 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DOWN, 1zu, 0zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 1zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DOWN, 1zu, 1zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 1zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::DOWN, 1zu, 2zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 1zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 3zu }, { node_layer::DOWN, 1zu, 3zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 1zu, 3zu })
        );
        // Down 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::DOWN, 2zu, 0zu } },
                    E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DOWN, 2zu, 0zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 2zu, 0zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::DOWN, 2zu, 1zu } },
                    E { { node_layer::DOWN, 1zu, 1zu }, { node_layer::DOWN, 2zu, 1zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 2zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::DOWN, 2zu, 2zu } },
                    E { { node_layer::DOWN, 1zu, 2zu }, { node_layer::DOWN, 2zu, 2zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 2zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 3zu }, { node_layer::DOWN, 2zu, 3zu } },
                    E { { node_layer::DOWN, 1zu, 3zu }, { node_layer::DOWN, 2zu, 3zu } }
                }
            ),
            to_inputs(N { node_layer::DOWN, 2zu, 3zu })
        );
        // Right 0,1 to 0,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::RIGHT, 0zu, 1zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 0zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::RIGHT, 0zu, 2zu } },
                    E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::RIGHT, 0zu, 2zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 0zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 0zu, 2zu }, { node_layer::RIGHT, 0zu, 3zu } },
                    E { { node_layer::RIGHT, 0zu, 2zu }, { node_layer::RIGHT, 0zu, 3zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 0zu, 3zu })
        );
        // Right 1,1 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 0zu }, { node_layer::RIGHT, 1zu, 1zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 1zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 1zu }, { node_layer::RIGHT, 1zu, 2zu } },
                    E { { node_layer::RIGHT, 1zu, 1zu }, { node_layer::RIGHT, 1zu, 2zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 1zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 1zu, 2zu }, { node_layer::RIGHT, 1zu, 3zu } },
                    E { { node_layer::RIGHT, 1zu, 2zu }, { node_layer::RIGHT, 1zu, 3zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 1zu, 3zu })
        );
        // Right 2,1 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 2zu, 0zu }, { node_layer::RIGHT, 2zu, 1zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 2zu, 1zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 2zu, 1zu }, { node_layer::RIGHT, 2zu, 2zu } },
                    E { { node_layer::RIGHT, 2zu, 1zu }, { node_layer::RIGHT, 2zu, 2zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 2zu, 2zu })
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    E { { node_layer::DIAGONAL, 2zu, 2zu }, { node_layer::RIGHT, 2zu, 3zu } },
                    E { { node_layer::RIGHT, 2zu, 2zu }, { node_layer::RIGHT, 2zu, 3zu } },
                }
            ),
            to_inputs(N { node_layer::RIGHT, 2zu, 3zu })
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputDegree) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // Diagonal 0,0 to 0,3
        EXPECT_EQ(3zu, g.get_out_degree(N { node_layer::DIAGONAL, 0zu, 0zu }));
        EXPECT_EQ(3zu, g.get_out_degree(N { node_layer::DIAGONAL, 0zu, 1zu }));
        EXPECT_EQ(3zu, g.get_out_degree(N { node_layer::DIAGONAL, 0zu, 2zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DIAGONAL, 0zu, 3zu }));
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(3zu, g.get_out_degree(N { node_layer::DIAGONAL, 1zu, 0zu }));
        EXPECT_EQ(3zu, g.get_out_degree(N { node_layer::DIAGONAL, 1zu, 1zu }));
        EXPECT_EQ(3zu, g.get_out_degree(N { node_layer::DIAGONAL, 1zu, 2zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DIAGONAL, 1zu, 3zu }));
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DIAGONAL, 2zu, 0zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DIAGONAL, 2zu, 1zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DIAGONAL, 2zu, 2zu }));
        EXPECT_EQ(0zu, g.get_out_degree(N { node_layer::DIAGONAL, 2zu, 3zu }));
        // Down 1,0 to 1,3
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::DOWN, 1zu, 0zu }));
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::DOWN, 1zu, 1zu }));
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::DOWN, 1zu, 2zu }));
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::DOWN, 1zu, 3zu }));
        // Down 2,0 to 2,3
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DOWN, 2zu, 0zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DOWN, 2zu, 1zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DOWN, 2zu, 2zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::DOWN, 2zu, 3zu }));
        // Right 0,1 to 0,3
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::RIGHT, 0zu, 1zu }));
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::RIGHT, 0zu, 2zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::RIGHT, 0zu, 3zu }));
        // Right 1,1 to 1,3
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::RIGHT, 1zu, 1zu }));
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::RIGHT, 1zu, 2zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::RIGHT, 1zu, 3zu }));
        // Right 2,1 to 2,3
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::RIGHT, 2zu, 1zu }));
        EXPECT_EQ(2zu, g.get_out_degree(N { node_layer::RIGHT, 2zu, 2zu }));
        EXPECT_EQ(1zu, g.get_out_degree(N { node_layer::RIGHT, 2zu, 3zu }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputDegree) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // Diagonal 0,0 to 0,3
        EXPECT_EQ(0zu, g.get_in_degree(N { node_layer::DIAGONAL, 0zu, 0zu }));
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DIAGONAL, 0zu, 1zu }));
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DIAGONAL, 0zu, 2zu }));
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DIAGONAL, 0zu, 3zu }));
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DIAGONAL, 1zu, 0zu }));
        EXPECT_EQ(3zu, g.get_in_degree(N { node_layer::DIAGONAL, 1zu, 1zu }));
        EXPECT_EQ(3zu, g.get_in_degree(N { node_layer::DIAGONAL, 1zu, 2zu }));
        EXPECT_EQ(3zu, g.get_in_degree(N { node_layer::DIAGONAL, 1zu, 3zu }));
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DIAGONAL, 2zu, 0zu }));
        EXPECT_EQ(3zu, g.get_in_degree(N { node_layer::DIAGONAL, 2zu, 1zu }));
        EXPECT_EQ(3zu, g.get_in_degree(N { node_layer::DIAGONAL, 2zu, 2zu }));
        EXPECT_EQ(3zu, g.get_in_degree(N { node_layer::DIAGONAL, 2zu, 3zu }));
        // Down 1,0 to 1,3
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DOWN, 1zu, 0zu }));
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DOWN, 1zu, 1zu }));
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DOWN, 1zu, 2zu }));
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::DOWN, 1zu, 3zu }));
        // Down 2,0 to 2,3
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::DOWN, 2zu, 0zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::DOWN, 2zu, 1zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::DOWN, 2zu, 2zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::DOWN, 2zu, 3zu }));
        // Right 0,1 to 0,3
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::RIGHT, 0zu, 1zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::RIGHT, 0zu, 2zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::RIGHT, 0zu, 3zu }));
        // Right 1,1 to 1,3
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::RIGHT, 1zu, 1zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::RIGHT, 1zu, 2zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::RIGHT, 1zu, 3zu }));
        // Right 2,1 to 2,3
        EXPECT_EQ(1zu, g.get_in_degree(N { node_layer::RIGHT, 2zu, 1zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::RIGHT, 2zu, 2zu }));
        EXPECT_EQ(2zu, g.get_in_degree(N { node_layer::RIGHT, 2zu, 3zu }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetEdgeData) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(1.0f64, g.get_edge_data(E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DIAGONAL, 1zu, 1zu } }));
        EXPECT_EQ(-1.0f64, g.get_edge_data(E { { node_layer::DIAGONAL, 0zu, 1zu }, { node_layer::DIAGONAL, 1zu, 2zu } }));
        EXPECT_EQ(0.0f64, g.get_edge_data(E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::DOWN, 1zu, 0zu } }));  // from match to extended gap
        EXPECT_EQ(0.0f64, g.get_edge_data(E { { node_layer::DIAGONAL, 0zu, 0zu }, { node_layer::RIGHT, 0zu, 1zu } }));  // from match to extended gap
        EXPECT_EQ(0.0f64, g.get_edge_data(E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::DIAGONAL, 0zu, 1zu } }));  // from extended gap to match
        EXPECT_EQ(0.0f64, g.get_edge_data(E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DIAGONAL, 1zu, 0zu } }));  // from extended gap to match
        EXPECT_EQ(0.1f64, g.get_edge_data(E { { node_layer::DOWN, 1zu, 0zu }, { node_layer::DOWN, 2zu, 0zu } }));  // from extended gap to extended gap
        EXPECT_EQ(0.1f64, g.get_edge_data(E { { node_layer::RIGHT, 0zu, 1zu }, { node_layer::RIGHT, 0zu, 2zu } }));  // from extended gap to extended gap
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

        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            (to_vector(g.slice_nodes(0u))),
            (std::vector<N> {
                N { node_layer::DIAGONAL, 0zu, 0zu },
                N { node_layer::RIGHT, 0zu, 1zu },
                N { node_layer::DIAGONAL, 0zu, 1zu },
                N { node_layer::RIGHT, 0zu, 2zu },
                N { node_layer::DIAGONAL, 0zu, 2zu }
            })
        );
        EXPECT_EQ(
            (to_vector(g.slice_nodes(1u))),
            (std::vector<N> {
                N { node_layer::DOWN, 1zu, 0zu },
                N { node_layer::DIAGONAL, 1zu, 0zu },
                N { node_layer::DOWN, 1zu, 1zu },
                N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 2zu }
            })
        );

        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DIAGONAL, 0zu, 0zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DOWN, 0zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::RIGHT, 0zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DIAGONAL, 0zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DOWN, 0zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::RIGHT, 0zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DIAGONAL, 1zu, 0zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DOWN, 1zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::RIGHT, 1zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DIAGONAL, 1zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DOWN, 1zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::RIGHT, 1zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { node_layer::DIAGONAL, 1zu, 2zu })), (std::vector<E> { }));

        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DIAGONAL, 0zu, 0zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DOWN, 0zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::RIGHT, 0zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DIAGONAL, 0zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DOWN, 0zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::RIGHT, 0zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DIAGONAL, 1zu, 0zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DOWN, 1zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::RIGHT, 1zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DIAGONAL, 1zu, 1zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DOWN, 1zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::RIGHT, 1zu, 2zu })), (std::vector<E> { }));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { node_layer::DIAGONAL, 1zu, 2zu })), (std::vector<E> { }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, SlicedWalkPartial) {
        auto to_vector {
            [](auto &&r) {
                auto it { r.begin() };
                std::vector<std::decay_t<decltype(*it)>> ret { };
                while (it != r.end()) {
                    ret.push_back(*it);
                    ++it;
                }
                return ret;
            }
        };

        std::string seq1 { "accd" };
        std::string seq2 { "accd" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            (to_vector(g.slice_nodes(0u, N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))),
            (std::vector<N> {
                N { node_layer::DIAGONAL, 0zu, 0zu },
                N { node_layer::RIGHT, 0zu, 1zu },
                N { node_layer::DIAGONAL, 0zu, 1zu },
                N { node_layer::RIGHT, 0zu, 2zu },
                N { node_layer::DIAGONAL, 0zu, 2zu },
                N { node_layer::RIGHT, 0zu, 3zu },
                N { node_layer::DIAGONAL, 0zu, 3zu },
                N { node_layer::RIGHT, 0zu, 4zu },
                N { node_layer::DIAGONAL, 0zu, 4zu },
            })
        );
        EXPECT_EQ(
            (to_vector(g.slice_nodes(4u, N { node_layer::DIAGONAL, 0zu, 0zu }, N { node_layer::DIAGONAL, 4zu, 4zu }))),
            (std::vector<N> {
                N { node_layer::DOWN, 4zu, 0zu },
                N { node_layer::DIAGONAL, 4zu, 0zu },
                N { node_layer::DOWN, 4zu, 1zu },
                N { node_layer::RIGHT, 4zu, 1zu },
                N { node_layer::DIAGONAL, 4zu, 1zu },
                N { node_layer::DOWN, 4zu, 2zu },
                N { node_layer::RIGHT, 4zu, 2zu },
                N { node_layer::DIAGONAL, 4zu, 2zu },
                N { node_layer::DOWN, 4zu, 3zu },
                N { node_layer::RIGHT, 4zu, 3zu },
                N { node_layer::DIAGONAL, 4zu, 3zu },
                N { node_layer::DOWN, 4zu, 4zu },
                N { node_layer::RIGHT, 4zu, 4zu },
                N { node_layer::DIAGONAL, 4zu, 4zu }
            })
        );

        EXPECT_EQ(
            (to_vector(g.slice_nodes(1u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::RIGHT, 4zu, 4zu }))),
            (std::vector<N> {
                // N { node_layer::DOWN, 1zu, 1zu },
                // N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 2zu },
                N { node_layer::DOWN, 1zu, 3zu },
                N { node_layer::RIGHT, 1zu, 3zu },
                N { node_layer::DIAGONAL, 1zu, 3zu },
                N { node_layer::DOWN, 1zu, 4zu },
                N { node_layer::RIGHT, 1zu, 4zu },
                N { node_layer::DIAGONAL, 1zu, 4zu }
            })
        );
        EXPECT_EQ(
            (to_vector(g.slice_nodes(2u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::RIGHT, 4zu, 4zu }))),
            (std::vector<N> {
                N { node_layer::DOWN, 2zu, 1zu },
                N { node_layer::RIGHT, 2zu, 1zu },
                N { node_layer::DIAGONAL, 2zu, 1zu },
                N { node_layer::DOWN, 2zu, 2zu },
                N { node_layer::RIGHT, 2zu, 2zu },
                N { node_layer::DIAGONAL, 2zu, 2zu },
                N { node_layer::DOWN, 2zu, 3zu },
                N { node_layer::RIGHT, 2zu, 3zu },
                N { node_layer::DIAGONAL, 2zu, 3zu },
                N { node_layer::DOWN, 2zu, 4zu },
                N { node_layer::RIGHT, 2zu, 4zu },
                N { node_layer::DIAGONAL, 2zu, 4zu }
            })
        );
        EXPECT_EQ(
            (to_vector(g.slice_nodes(4u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::RIGHT, 4zu, 4zu }))),
            (std::vector<N> {
                N { node_layer::DOWN, 4zu, 1zu },
                N { node_layer::RIGHT, 4zu, 1zu },
                N { node_layer::DIAGONAL, 4zu, 1zu },
                N { node_layer::DOWN, 4zu, 2zu },
                N { node_layer::RIGHT, 4zu, 2zu },
                N { node_layer::DIAGONAL, 4zu, 2zu },
                N { node_layer::DOWN, 4zu, 3zu },
                N { node_layer::RIGHT, 4zu, 3zu },
                N { node_layer::DIAGONAL, 4zu, 3zu },
                N { node_layer::DOWN, 4zu, 4zu },
                N { node_layer::RIGHT, 4zu, 4zu },
                // N { node_layer::DIAGONAL, 4zu, 4zu },
            })
        );

        EXPECT_EQ(
            (to_vector(g.slice_nodes(1u, N { node_layer::DIAGONAL, 1zu, 1zu }, N { node_layer::RIGHT, 1zu, 4zu }))),
            (std::vector<N> {
                // N { node_layer::DOWN, 1zu, 1zu },
                // N { node_layer::RIGHT, 1zu, 1zu },
                N { node_layer::DIAGONAL, 1zu, 1zu },
                N { node_layer::DOWN, 1zu, 2zu },
                N { node_layer::RIGHT, 1zu, 2zu },
                N { node_layer::DIAGONAL, 1zu, 2zu },
                N { node_layer::DOWN, 1zu, 3zu },
                N { node_layer::RIGHT, 1zu, 3zu },
                N { node_layer::DIAGONAL, 1zu, 3zu },
                N { node_layer::DOWN, 1zu, 4zu },
                N { node_layer::RIGHT, 1zu, 4zu },
                // N { node_layer::DIAGONAL, 1zu, 4zu }
            })
        );
    }


    bool walk(auto& g, auto current, auto expected) {
        if (current == expected) {
            return true;
        }
        for (const auto& [_, __, to_node, ___] : g.get_outputs_full(current)) {
            if (walk(g, to_node, expected) == true) {
                return true;
            }
        }
        return false;
    };

    TEST(PairwiseExtendedGapAlignmentGraphTest, IsReachableTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-0.1f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaalaa" };
        std::string seq2 { "lv" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        for (const N& n1 : g.get_nodes()) {
            for (const N& n2 : g.get_nodes()) {
                const auto& [n1_layer, n1_down, n1_right] { n1 };
                const auto& [n2_layer, n2_down, n2_right] { n2 };
                // std::cout << n1_down << '/' << n1_right << '/' << static_cast<int>(n1_layer) << " to " << n2_down << '/' << n2_right << '/' << static_cast<int>(n2_layer) << ' ' << std::endl;
                bool walk_val { walk(g, n1, n2) };
                bool is_reachable_val { g.is_reachable(n1, n2) };
                EXPECT_EQ(is_reachable_val, walk_val);
            }
        }
    }
}