#include <cstddef>
#include <algorithm>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto substitution_scorer { simple_scorer<char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto initial_gap_scorer { simple_scorer<char, char, std::float64_t>::create_gap(0.0f64) };
    auto extended_gap_scorer { simple_scorer<char, char, std::float64_t>::create_gap(0.1f64) };
    auto freeride_scorer { simple_scorer<char, char, std::float64_t>::create_freeride() };

    TEST(PairwiseExtendedGapAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_extended_gap_alignment_graph<std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ListNodes) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename std::remove_reference_t<decltype(g)>::N;

        std::set<N> actual {};
        for (const auto &n : g.get_nodes()) {
            actual.insert(n);
        }
        EXPECT_EQ(
            actual,
            (std::set {
                N {layer::DIAGONAL, 0zu, 0zu},
                N {layer::DIAGONAL, 0zu, 1zu},
                N {layer::DIAGONAL, 0zu, 2zu},
                N {layer::DIAGONAL, 0zu, 3zu},
                N {layer::DIAGONAL, 1zu, 0zu},
                N {layer::DIAGONAL, 1zu, 1zu},
                N {layer::DIAGONAL, 1zu, 2zu},
                N {layer::DIAGONAL, 1zu, 3zu},
                N {layer::DIAGONAL, 2zu, 0zu},
                N {layer::DIAGONAL, 2zu, 1zu},
                N {layer::DIAGONAL, 2zu, 2zu},
                N {layer::DIAGONAL, 2zu, 3zu},
                N {layer::DOWN, 1zu, 0zu},
                N {layer::DOWN, 1zu, 1zu},
                N {layer::DOWN, 1zu, 2zu},
                N {layer::DOWN, 1zu, 3zu},
                N {layer::DOWN, 2zu, 0zu},
                N {layer::DOWN, 2zu, 1zu},
                N {layer::DOWN, 2zu, 2zu},
                N {layer::DOWN, 2zu, 3zu},
                N {layer::RIGHT, 0zu, 1zu},
                N {layer::RIGHT, 1zu, 1zu},
                N {layer::RIGHT, 2zu, 1zu},
                N {layer::RIGHT, 0zu, 2zu},
                N {layer::RIGHT, 1zu, 2zu},
                N {layer::RIGHT, 2zu, 2zu},
                N {layer::RIGHT, 0zu, 3zu},
                N {layer::RIGHT, 1zu, 3zu},
                N {layer::RIGHT, 2zu, 3zu}
            })
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, ListEdges) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using N = typename std::remove_reference_t<decltype(g)>::N;
        using E = typename std::remove_reference_t<decltype(g)>::E;

        auto e = g.get_edges();
        std::vector<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
        for (auto _e : e) {
            actual.push_back(_e);
        }
        std::ranges::sort(actual);
        std::vector<E> expected {
            E { N {layer::DIAGONAL, 0zu, 0zu}, N {layer::DIAGONAL, 1zu, 1zu} },
            E { N {layer::DIAGONAL, 0zu, 1zu}, N {layer::DIAGONAL, 1zu, 2zu} },
            E { N {layer::DIAGONAL, 0zu, 2zu}, N {layer::DIAGONAL, 1zu, 3zu} },
            E { N {layer::DIAGONAL, 1zu, 0zu}, N {layer::DIAGONAL, 2zu, 1zu} },
            E { N {layer::DIAGONAL, 1zu, 1zu}, N {layer::DIAGONAL, 2zu, 2zu} },
            E { N {layer::DIAGONAL, 1zu, 2zu}, N {layer::DIAGONAL, 2zu, 3zu} },
            E { N {layer::DIAGONAL, 0zu, 0zu}, N {layer::RIGHT, 0zu, 1zu} },
            E { N {layer::DIAGONAL, 0zu, 1zu}, N {layer::RIGHT, 0zu, 2zu} },
            E { N {layer::DIAGONAL, 0zu, 2zu}, N {layer::RIGHT, 0zu, 3zu} },
            E { N {layer::DIAGONAL, 1zu, 0zu}, N {layer::RIGHT, 1zu, 1zu} },
            E { N {layer::DIAGONAL, 1zu, 1zu}, N {layer::RIGHT, 1zu, 2zu} },
            E { N {layer::DIAGONAL, 1zu, 2zu}, N {layer::RIGHT, 1zu, 3zu} },
            E { N {layer::DIAGONAL, 2zu, 0zu}, N {layer::RIGHT, 2zu, 1zu} },
            E { N {layer::DIAGONAL, 2zu, 1zu}, N {layer::RIGHT, 2zu, 2zu} },
            E { N {layer::DIAGONAL, 2zu, 2zu}, N {layer::RIGHT, 2zu, 3zu} },
            E { N {layer::DIAGONAL, 0zu, 0zu}, N {layer::DOWN, 1zu, 0zu} },
            E { N {layer::DIAGONAL, 1zu, 0zu}, N {layer::DOWN, 2zu, 0zu} },
            E { N {layer::DIAGONAL, 0zu, 1zu}, N {layer::DOWN, 1zu, 1zu} },
            E { N {layer::DIAGONAL, 1zu, 1zu}, N {layer::DOWN, 2zu, 1zu} },
            E { N {layer::DIAGONAL, 0zu, 2zu}, N {layer::DOWN, 1zu, 2zu} },
            E { N {layer::DIAGONAL, 1zu, 2zu}, N {layer::DOWN, 2zu, 2zu} },
            E { N {layer::DIAGONAL, 0zu, 3zu}, N {layer::DOWN, 1zu, 3zu} },
            E { N {layer::DIAGONAL, 1zu, 3zu}, N {layer::DOWN, 2zu, 3zu} },
            E { N {layer::DOWN, 1zu, 0zu}, N {layer::DIAGONAL, 1zu, 0zu} },
            E { N {layer::DOWN, 1zu, 1zu}, N {layer::DIAGONAL, 1zu, 1zu} },
            E { N {layer::DOWN, 1zu, 2zu}, N {layer::DIAGONAL, 1zu, 2zu} },
            E { N {layer::DOWN, 1zu, 3zu}, N {layer::DIAGONAL, 1zu, 3zu} },
            E { N {layer::DOWN, 2zu, 0zu}, N {layer::DIAGONAL, 2zu, 0zu} },
            E { N {layer::DOWN, 2zu, 1zu}, N {layer::DIAGONAL, 2zu, 1zu} },
            E { N {layer::DOWN, 2zu, 2zu}, N {layer::DIAGONAL, 2zu, 2zu} },
            E { N {layer::DOWN, 2zu, 3zu}, N {layer::DIAGONAL, 2zu, 3zu} },
            E { N {layer::DOWN, 1zu, 0zu}, N {layer::DOWN, 2zu, 0zu} },
            E { N {layer::DOWN, 1zu, 1zu}, N {layer::DOWN, 2zu, 1zu} },
            E { N {layer::DOWN, 1zu, 2zu}, N {layer::DOWN, 2zu, 2zu} },
            E { N {layer::DOWN, 1zu, 3zu}, N {layer::DOWN, 2zu, 3zu} },
            E { N {layer::RIGHT, 0zu, 1zu}, N {layer::DIAGONAL, 0zu, 1zu} },
            E { N {layer::RIGHT, 0zu, 2zu}, N {layer::DIAGONAL, 0zu, 2zu} },
            E { N {layer::RIGHT, 0zu, 3zu}, N {layer::DIAGONAL, 0zu, 3zu} },
            E { N {layer::RIGHT, 1zu, 1zu}, N {layer::DIAGONAL, 1zu, 1zu} },
            E { N {layer::RIGHT, 1zu, 2zu}, N {layer::DIAGONAL, 1zu, 2zu} },
            E { N {layer::RIGHT, 1zu, 3zu}, N {layer::DIAGONAL, 1zu, 3zu} },
            E { N {layer::RIGHT, 2zu, 1zu}, N {layer::DIAGONAL, 2zu, 1zu} },
            E { N {layer::RIGHT, 2zu, 2zu}, N {layer::DIAGONAL, 2zu, 2zu} },
            E { N {layer::RIGHT, 2zu, 3zu}, N {layer::DIAGONAL, 2zu, 3zu} },
            E { N {layer::RIGHT, 0zu, 1zu}, N {layer::RIGHT, 0zu, 2zu} },
            E { N {layer::RIGHT, 0zu, 2zu}, N {layer::RIGHT, 0zu, 3zu} },
            E { N {layer::RIGHT, 1zu, 1zu}, N {layer::RIGHT, 1zu, 2zu} },
            E { N {layer::RIGHT, 1zu, 2zu}, N {layer::RIGHT, 1zu, 3zu} },
            E { N {layer::RIGHT, 2zu, 1zu}, N {layer::RIGHT, 2zu, 2zu} },
            E { N {layer::RIGHT, 2zu, 2zu}, N {layer::RIGHT, 2zu, 3zu} }
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
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 0zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::DIAGONAL, 0zu, 4zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 1zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::DIAGONAL, 1zu, 4zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::DIAGONAL, 2zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::DIAGONAL, 2zu, 4zu}));
        EXPECT_FALSE(g.has_node({layer::DIAGONAL, 3zu, 3zu}));

        EXPECT_FALSE(g.has_node({layer::DOWN, 0zu, 0zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 0zu, 1zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 0zu, 2zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 0zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 0zu, 4zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 1zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 1zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 1zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 1zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 1zu, 4zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 2zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 2zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 2zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::DOWN, 2zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 2zu, 4zu}));
        EXPECT_FALSE(g.has_node({layer::DOWN, 3zu, 3zu}));

        EXPECT_FALSE(g.has_node({layer::RIGHT, 0zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 0zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 0zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 0zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::RIGHT, 0zu, 4zu}));
        EXPECT_FALSE(g.has_node({layer::RIGHT, 1zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 1zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 1zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 1zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::RIGHT, 1zu, 4zu}));
        EXPECT_FALSE(g.has_node({layer::RIGHT, 2zu, 0zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 2zu, 1zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 2zu, 2zu}));
        EXPECT_TRUE(g.has_node({layer::RIGHT, 2zu, 3zu}));
        EXPECT_FALSE(g.has_node({layer::RIGHT, 2zu, 4zu}));
        EXPECT_FALSE(g.has_node({layer::RIGHT, 3zu, 3zu}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 0zu}, {layer::RIGHT, 0zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 1zu}, {layer::RIGHT, 0zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 2zu}, {layer::RIGHT, 0zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 0zu, 3zu}, {layer::RIGHT, 0zu, 4zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 0zu}, {layer::RIGHT, 1zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 1zu}, {layer::RIGHT, 1zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 2zu}, {layer::RIGHT, 1zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 1zu, 3zu}, {layer::RIGHT, 1zu, 4zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 2zu, 0zu}, {layer::RIGHT, 2zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 2zu, 1zu}, {layer::RIGHT, 2zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 2zu, 2zu}, {layer::RIGHT, 2zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2zu, 3zu}, {layer::RIGHT, 2zu, 4zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 3zu, 0zu}, {layer::RIGHT, 3zu, 1zu}}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 0zu}, {layer::DOWN, 1zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 0zu}, {layer::DOWN, 2zu, 0zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2zu, 0zu}, {layer::DOWN, 3zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 1zu}, {layer::DOWN, 1zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 1zu}, {layer::DOWN, 2zu, 1zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2zu, 1zu}, {layer::DOWN, 3zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 2zu}, {layer::DOWN, 1zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 2zu}, {layer::DOWN, 2zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2zu, 2zu}, {layer::DOWN, 3zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 0zu, 3zu}, {layer::DOWN, 1zu, 3zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DIAGONAL, 1zu, 3zu}, {layer::DOWN, 2zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 2zu, 3zu}, {layer::DOWN, 3zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DIAGONAL, 3zu, 0zu}, {layer::DOWN, 4zu, 0zu}}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DiagEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 0zu, 0zu}, {layer::DIAGONAL, 1zu, 1zu} }));
        EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 0zu, 1zu}, {layer::DIAGONAL, 1zu, 2zu} }));
        EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 0zu, 2zu}, {layer::DIAGONAL, 1zu, 3zu} }));
        EXPECT_FALSE(g.has_edge({ {layer::DIAGONAL, 0zu, 3zu}, {layer::DIAGONAL, 1zu, 4zu} }));
        EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 1zu, 0zu}, {layer::DIAGONAL, 2zu, 1zu} }));
        EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 1zu, 1zu}, {layer::DIAGONAL, 2zu, 2zu} }));
        EXPECT_TRUE(g.has_edge({ {layer::DIAGONAL, 1zu, 2zu}, {layer::DIAGONAL, 2zu, 3zu} }));
        EXPECT_FALSE(g.has_edge({ {layer::DIAGONAL, 1zu, 3zu}, {layer::DIAGONAL, 2zu, 4zu} }));
        EXPECT_FALSE(g.has_edge({ {layer::DIAGONAL, 2zu, 0zu}, {layer::DIAGONAL, 3zu, 1zu} }));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, DownFreeRidgeEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_FALSE(g.has_edge({{layer::DOWN, 0zu, 0zu}, {layer::DIAGONAL, 0zu, 0zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 0zu, 1zu}, {layer::DIAGONAL, 0zu, 1zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 0zu, 2zu}, {layer::DIAGONAL, 0zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 0zu, 3zu}, {layer::DIAGONAL, 0zu, 3zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 1zu, 0zu}, {layer::DIAGONAL, 1zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 1zu, 1zu}, {layer::DIAGONAL, 1zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 1zu, 2zu}, {layer::DIAGONAL, 1zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 1zu, 3zu}, {layer::DIAGONAL, 1zu, 3zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 2zu, 0zu}, {layer::DIAGONAL, 2zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 2zu, 1zu}, {layer::DIAGONAL, 2zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 2zu, 2zu}, {layer::DIAGONAL, 2zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::DOWN, 2zu, 3zu}, {layer::DIAGONAL, 2zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 3zu, 0zu}, {layer::DIAGONAL, 3zu, 0zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 3zu, 1zu}, {layer::DIAGONAL, 3zu, 1zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 3zu, 2zu}, {layer::DIAGONAL, 3zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{layer::DOWN, 3zu, 3zu}, {layer::DIAGONAL, 3zu, 3zu}}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, RightFreeRidgeEdgesExist) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 0zu, 0zu}, {layer::DIAGONAL, 0zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 0zu, 1zu}, {layer::DIAGONAL, 0zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 0zu, 2zu}, {layer::DIAGONAL, 0zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 0zu, 3zu}, {layer::DIAGONAL, 0zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 1zu, 0zu}, {layer::DIAGONAL, 1zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 1zu, 1zu}, {layer::DIAGONAL, 1zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 1zu, 2zu}, {layer::DIAGONAL, 1zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 1zu, 3zu}, {layer::DIAGONAL, 1zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 2zu, 0zu}, {layer::DIAGONAL, 2zu, 0zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 2zu, 1zu}, {layer::DIAGONAL, 2zu, 1zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 2zu, 2zu}, {layer::DIAGONAL, 2zu, 2zu}}));
        EXPECT_TRUE(g.has_edge({{layer::RIGHT, 2zu, 3zu}, {layer::DIAGONAL, 2zu, 3zu}}));
        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3zu, 0zu}, {layer::DIAGONAL, 3zu, 0zu}}));
        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3zu, 1zu}, {layer::DIAGONAL, 3zu, 1zu}}));
        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3zu, 2zu}, {layer::DIAGONAL, 3zu, 2zu}}));
        EXPECT_FALSE(g.has_edge({{layer::RIGHT, 3zu, 3zu}, {layer::DIAGONAL, 3zu, 3zu}}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputs) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

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
                    {{layer::DIAGONAL, 0zu, 0zu}, {layer::DIAGONAL, 1zu, 1zu}},
                    {{layer::DIAGONAL, 0zu, 0zu}, {layer::DOWN, 1zu, 0zu}},
                    {{layer::DIAGONAL, 0zu, 0zu}, {layer::RIGHT, 0zu, 1zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 0zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 1zu}, {layer::DIAGONAL, 1zu, 2zu}},
                    {{layer::DIAGONAL, 0zu, 1zu}, {layer::DOWN, 1zu, 1zu}},
                    {{layer::DIAGONAL, 0zu, 1zu}, {layer::RIGHT, 0zu, 2zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 0zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 2zu}, {layer::DIAGONAL, 1zu, 3zu}},
                    {{layer::DIAGONAL, 0zu, 2zu}, {layer::DOWN, 1zu, 2zu}},
                    {{layer::DIAGONAL, 0zu, 2zu}, {layer::RIGHT, 0zu, 3zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 0zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 3zu}, {layer::DOWN, 1zu, 3zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 0zu, 3zu})
        );
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 0zu}, {layer::DIAGONAL, 2zu, 1zu}},
                    {{layer::DIAGONAL, 1zu, 0zu}, {layer::DOWN, 2zu, 0zu}},
                    {{layer::DIAGONAL, 1zu, 0zu}, {layer::RIGHT, 1zu, 1zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 1zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 1zu}, {layer::DIAGONAL, 2zu, 2zu}},
                    {{layer::DIAGONAL, 1zu, 1zu}, {layer::DOWN, 2zu, 1zu}},
                    {{layer::DIAGONAL, 1zu, 1zu}, {layer::RIGHT, 1zu, 2zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 1zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 2zu}, {layer::DIAGONAL, 2zu, 3zu}},
                    {{layer::DIAGONAL, 1zu, 2zu}, {layer::DOWN, 2zu, 2zu}},
                    {{layer::DIAGONAL, 1zu, 2zu}, {layer::RIGHT, 1zu, 3zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 1zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 3zu}, {layer::DOWN, 2zu, 3zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 1zu, 3zu})
        );
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 2zu, 0zu}, {layer::RIGHT, 2zu, 1zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 2zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 2zu, 1zu}, {layer::RIGHT, 2zu, 2zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 2zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 2zu, 2zu}, {layer::RIGHT, 2zu, 3zu}}
                }
            ),
            to_outputs({layer::DIAGONAL, 2zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                }
            ),
            to_outputs({layer::DIAGONAL, 2zu, 3zu})
        );
        // Down 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 1zu, 0zu}, {layer::DIAGONAL, 1zu, 0zu}},
                    {{layer::DOWN, 1zu, 0zu}, {layer::DOWN, 2zu, 0zu}},
                }
            ),
            to_outputs({layer::DOWN, 1zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 1zu, 1zu}, {layer::DIAGONAL, 1zu, 1zu}},
                    {{layer::DOWN, 1zu, 1zu}, {layer::DOWN, 2zu, 1zu}},
                }
            ),
            to_outputs({layer::DOWN, 1zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 1zu, 2zu}, {layer::DIAGONAL, 1zu, 2zu}},
                    {{layer::DOWN, 1zu, 2zu}, {layer::DOWN, 2zu, 2zu}},
                }
            ),
            to_outputs({layer::DOWN, 1zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 1zu, 3zu}, {layer::DIAGONAL, 1zu, 3zu}},
                    {{layer::DOWN, 1zu, 3zu}, {layer::DOWN, 2zu, 3zu}},
                }
            ),
            to_outputs({layer::DOWN, 1zu, 3zu})
        );
        // Down 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 2zu, 0zu}, {layer::DIAGONAL, 2zu, 0zu}},
                }
            ),
            to_outputs({layer::DOWN, 2zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 2zu, 1zu}, {layer::DIAGONAL, 2zu, 1zu}},
                }
            ),
            to_outputs({layer::DOWN, 2zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 2zu, 2zu}, {layer::DIAGONAL, 2zu, 2zu}},
                }
            ),
            to_outputs({layer::DOWN, 2zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 2zu, 3zu}, {layer::DIAGONAL, 2zu, 3zu}},
                }
            ),
            to_outputs({layer::DOWN, 2zu, 3zu})
        );
        // Right 0,1 to 0,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 0zu, 1zu}, {layer::DIAGONAL, 0zu, 1zu}},
                    {{layer::RIGHT, 0zu, 1zu}, {layer::RIGHT, 0zu, 2zu}},
                }
            ),
            to_outputs({layer::RIGHT, 0zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 0zu, 2zu}, {layer::DIAGONAL, 0zu, 2zu}},
                    {{layer::RIGHT, 0zu, 2zu}, {layer::RIGHT, 0zu, 3zu}},
                }
            ),
            to_outputs({layer::RIGHT, 0zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 0zu, 3zu}, {layer::DIAGONAL, 0zu, 3zu}},
                }
            ),
            to_outputs({layer::RIGHT, 0zu, 3zu})
        );
        // Right 1,1 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 1zu, 1zu}, {layer::DIAGONAL, 1zu, 1zu}},
                    {{layer::RIGHT, 1zu, 1zu}, {layer::RIGHT, 1zu, 2zu}},
                }
            ),
            to_outputs({layer::RIGHT, 1zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 1zu, 2zu}, {layer::DIAGONAL, 1zu, 2zu}},
                    {{layer::RIGHT, 1zu, 2zu}, {layer::RIGHT, 1zu, 3zu}},
                }
            ),
            to_outputs({layer::RIGHT, 1zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 1zu, 3zu}, {layer::DIAGONAL, 1zu, 3zu}},
                }
            ),
            to_outputs({layer::RIGHT, 1zu, 3zu})
        );
        // Right 2,1 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 2zu, 1zu}, {layer::DIAGONAL, 2zu, 1zu}},
                    {{layer::RIGHT, 2zu, 1zu}, {layer::RIGHT, 2zu, 2zu}},
                }
            ),
            to_outputs({layer::RIGHT, 2zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 2zu, 2zu}, {layer::DIAGONAL, 2zu, 2zu}},
                    {{layer::RIGHT, 2zu, 2zu}, {layer::RIGHT, 2zu, 3zu}},
                }
            ),
            to_outputs({layer::RIGHT, 2zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 2zu, 3zu}, {layer::DIAGONAL, 2zu, 3zu}},
                }
            ),
            to_outputs({layer::RIGHT, 2zu, 3zu})
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputs) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

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
            to_inputs({layer::DIAGONAL, 0zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 0zu, 1zu}, {layer::DIAGONAL, 0zu, 1zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 0zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 0zu, 2zu}, {layer::DIAGONAL, 0zu, 2zu}},
                }
            ),
            to_inputs({layer::DIAGONAL, 0zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::RIGHT, 0zu, 3zu}, {layer::DIAGONAL, 0zu, 3zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 0zu, 3zu})
        );
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 1zu, 0zu}, {layer::DIAGONAL, 1zu, 0zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 1zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 0zu}, {layer::DIAGONAL, 1zu, 1zu}},
                    {{layer::DOWN, 1zu, 1zu}, {layer::DIAGONAL, 1zu, 1zu}},
                    {{layer::RIGHT, 1zu, 1zu}, {layer::DIAGONAL, 1zu, 1zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 1zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 1zu}, {layer::DIAGONAL, 1zu, 2zu}},
                    {{layer::DOWN, 1zu, 2zu}, {layer::DIAGONAL, 1zu, 2zu}},
                    {{layer::RIGHT, 1zu, 2zu}, {layer::DIAGONAL, 1zu, 2zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 1zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 2zu}, {layer::DIAGONAL, 1zu, 3zu}},
                    {{layer::DOWN, 1zu, 3zu}, {layer::DIAGONAL, 1zu, 3zu}},
                    {{layer::RIGHT, 1zu, 3zu}, {layer::DIAGONAL, 1zu, 3zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 1zu, 3zu})
        );
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DOWN, 2zu, 0zu}, {layer::DIAGONAL, 2zu, 0zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 2zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 0zu}, {layer::DIAGONAL, 2zu, 1zu}},
                    {{layer::DOWN, 2zu, 1zu}, {layer::DIAGONAL, 2zu, 1zu}},
                    {{layer::RIGHT, 2zu, 1zu}, {layer::DIAGONAL, 2zu, 1zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 2zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 1zu}, {layer::DIAGONAL, 2zu, 2zu}},
                    {{layer::DOWN, 2zu, 2zu}, {layer::DIAGONAL, 2zu, 2zu}},
                    {{layer::RIGHT, 2zu, 2zu}, {layer::DIAGONAL, 2zu, 2zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 2zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 2zu}, {layer::DIAGONAL, 2zu, 3zu}},
                    {{layer::DOWN, 2zu, 3zu}, {layer::DIAGONAL, 2zu, 3zu}},
                    {{layer::RIGHT, 2zu, 3zu}, {layer::DIAGONAL, 2zu, 3zu}}
                }
            ),
            to_inputs({layer::DIAGONAL, 2zu, 3zu})
        );
        // Down 1,0 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 0zu}, {layer::DOWN, 1zu, 0zu}}
                }
            ),
            to_inputs({layer::DOWN, 1zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 1zu}, {layer::DOWN, 1zu, 1zu}}
                }
            ),
            to_inputs({layer::DOWN, 1zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 2zu}, {layer::DOWN, 1zu, 2zu}}
                }
            ),
            to_inputs({layer::DOWN, 1zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 3zu}, {layer::DOWN, 1zu, 3zu}}
                }
            ),
            to_inputs({layer::DOWN, 1zu, 3zu})
        );
        // Down 2,0 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 0zu}, {layer::DOWN, 2zu, 0zu}},
                    {{layer::DOWN, 1zu, 0zu}, {layer::DOWN, 2zu, 0zu}}
                }
            ),
            to_inputs({layer::DOWN, 2zu, 0zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 1zu}, {layer::DOWN, 2zu, 1zu}},
                    {{layer::DOWN, 1zu, 1zu}, {layer::DOWN, 2zu, 1zu}}
                }
            ),
            to_inputs({layer::DOWN, 2zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 2zu}, {layer::DOWN, 2zu, 2zu}},
                    {{layer::DOWN, 1zu, 2zu}, {layer::DOWN, 2zu, 2zu}}
                }
            ),
            to_inputs({layer::DOWN, 2zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 3zu}, {layer::DOWN, 2zu, 3zu}},
                    {{layer::DOWN, 1zu, 3zu}, {layer::DOWN, 2zu, 3zu}}
                }
            ),
            to_inputs({layer::DOWN, 2zu, 3zu})
        );
        // Right 0,1 to 0,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 0zu}, {layer::RIGHT, 0zu, 1zu}},
                }
            ),
            to_inputs({layer::RIGHT, 0zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 1zu}, {layer::RIGHT, 0zu, 2zu}},
                    {{layer::RIGHT, 0zu, 1zu}, {layer::RIGHT, 0zu, 2zu}},
                }
            ),
            to_inputs({layer::RIGHT, 0zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 0zu, 2zu}, {layer::RIGHT, 0zu, 3zu}},
                    {{layer::RIGHT, 0zu, 2zu}, {layer::RIGHT, 0zu, 3zu}},
                }
            ),
            to_inputs({layer::RIGHT, 0zu, 3zu})
        );
        // Right 1,1 to 1,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 0zu}, {layer::RIGHT, 1zu, 1zu}},
                }
            ),
            to_inputs({layer::RIGHT, 1zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 1zu}, {layer::RIGHT, 1zu, 2zu}},
                    {{layer::RIGHT, 1zu, 1zu}, {layer::RIGHT, 1zu, 2zu}},
                }
            ),
            to_inputs({layer::RIGHT, 1zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 1zu, 2zu}, {layer::RIGHT, 1zu, 3zu}},
                    {{layer::RIGHT, 1zu, 2zu}, {layer::RIGHT, 1zu, 3zu}},
                }
            ),
            to_inputs({layer::RIGHT, 1zu, 3zu})
        );
        // Right 2,1 to 2,3
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 2zu, 0zu}, {layer::RIGHT, 2zu, 1zu}},
                }
            ),
            to_inputs({layer::RIGHT, 2zu, 1zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 2zu, 1zu}, {layer::RIGHT, 2zu, 2zu}},
                    {{layer::RIGHT, 2zu, 1zu}, {layer::RIGHT, 2zu, 2zu}},
                }
            ),
            to_inputs({layer::RIGHT, 2zu, 2zu})
        );
        EXPECT_EQ(
            (
                std::set<E> {
                    {{layer::DIAGONAL, 2zu, 2zu}, {layer::RIGHT, 2zu, 3zu}},
                    {{layer::RIGHT, 2zu, 2zu}, {layer::RIGHT, 2zu, 3zu}},
                }
            ),
            to_inputs({layer::RIGHT, 2zu, 3zu})
        );
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetOutputDegree) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        // Diagonal 0,0 to 0,3
        EXPECT_EQ(3zu, g.get_out_degree({layer::DIAGONAL, 0zu, 0zu}));
        EXPECT_EQ(3zu, g.get_out_degree({layer::DIAGONAL, 0zu, 1zu}));
        EXPECT_EQ(3zu, g.get_out_degree({layer::DIAGONAL, 0zu, 2zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DIAGONAL, 0zu, 3zu}));
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(3zu, g.get_out_degree({layer::DIAGONAL, 1zu, 0zu}));
        EXPECT_EQ(3zu, g.get_out_degree({layer::DIAGONAL, 1zu, 1zu}));
        EXPECT_EQ(3zu, g.get_out_degree({layer::DIAGONAL, 1zu, 2zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DIAGONAL, 1zu, 3zu}));
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(1zu, g.get_out_degree({layer::DIAGONAL, 2zu, 0zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DIAGONAL, 2zu, 1zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DIAGONAL, 2zu, 2zu}));
        EXPECT_EQ(0zu, g.get_out_degree({layer::DIAGONAL, 2zu, 3zu}));
        // Down 1,0 to 1,3
        EXPECT_EQ(2zu, g.get_out_degree({layer::DOWN, 1zu, 0zu}));
        EXPECT_EQ(2zu, g.get_out_degree({layer::DOWN, 1zu, 1zu}));
        EXPECT_EQ(2zu, g.get_out_degree({layer::DOWN, 1zu, 2zu}));
        EXPECT_EQ(2zu, g.get_out_degree({layer::DOWN, 1zu, 3zu}));
        // Down 2,0 to 2,3
        EXPECT_EQ(1zu, g.get_out_degree({layer::DOWN, 2zu, 0zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DOWN, 2zu, 1zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DOWN, 2zu, 2zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::DOWN, 2zu, 3zu}));
        // Right 0,1 to 0,3
        EXPECT_EQ(2zu, g.get_out_degree({layer::RIGHT, 0zu, 1zu}));
        EXPECT_EQ(2zu, g.get_out_degree({layer::RIGHT, 0zu, 2zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::RIGHT, 0zu, 3zu}));
        // Right 1,1 to 1,3
        EXPECT_EQ(2zu, g.get_out_degree({layer::RIGHT, 1zu, 1zu}));
        EXPECT_EQ(2zu, g.get_out_degree({layer::RIGHT, 1zu, 2zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::RIGHT, 1zu, 3zu}));
        // Right 2,1 to 2,3
        EXPECT_EQ(2zu, g.get_out_degree({layer::RIGHT, 2zu, 1zu}));
        EXPECT_EQ(2zu, g.get_out_degree({layer::RIGHT, 2zu, 2zu}));
        EXPECT_EQ(1zu, g.get_out_degree({layer::RIGHT, 2zu, 3zu}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetInputDegree) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        // Diagonal 0,0 to 0,3
        EXPECT_EQ(0zu, g.get_in_degree({layer::DIAGONAL, 0zu, 0zu}));
        EXPECT_EQ(1zu, g.get_in_degree({layer::DIAGONAL, 0zu, 1zu}));
        EXPECT_EQ(1zu, g.get_in_degree({layer::DIAGONAL, 0zu, 2zu}));
        EXPECT_EQ(1zu, g.get_in_degree({layer::DIAGONAL, 0zu, 3zu}));
        // Diagonal 1,0 to 1,3
        EXPECT_EQ(1zu, g.get_in_degree({layer::DIAGONAL, 1zu, 0zu}));
        EXPECT_EQ(3zu, g.get_in_degree({layer::DIAGONAL, 1zu, 1zu}));
        EXPECT_EQ(3zu, g.get_in_degree({layer::DIAGONAL, 1zu, 2zu}));
        EXPECT_EQ(3zu, g.get_in_degree({layer::DIAGONAL, 1zu, 3zu}));
        // Diagonal 2,0 to 2,3
        EXPECT_EQ(1zu, g.get_in_degree({layer::DIAGONAL, 2zu, 0zu}));
        EXPECT_EQ(3zu, g.get_in_degree({layer::DIAGONAL, 2zu, 1zu}));
        EXPECT_EQ(3zu, g.get_in_degree({layer::DIAGONAL, 2zu, 2zu}));
        EXPECT_EQ(3zu, g.get_in_degree({layer::DIAGONAL, 2zu, 3zu}));
        // Down 1,0 to 1,3
        EXPECT_EQ(1zu, g.get_in_degree({layer::DOWN, 1zu, 0zu}));
        EXPECT_EQ(1zu, g.get_in_degree({layer::DOWN, 1zu, 1zu}));
        EXPECT_EQ(1zu, g.get_in_degree({layer::DOWN, 1zu, 2zu}));
        EXPECT_EQ(1zu, g.get_in_degree({layer::DOWN, 1zu, 3zu}));
        // Down 2,0 to 2,3
        EXPECT_EQ(2zu, g.get_in_degree({layer::DOWN, 2zu, 0zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::DOWN, 2zu, 1zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::DOWN, 2zu, 2zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::DOWN, 2zu, 3zu}));
        // Right 0,1 to 0,3
        EXPECT_EQ(1zu, g.get_in_degree({layer::RIGHT, 0zu, 1zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::RIGHT, 0zu, 2zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::RIGHT, 0zu, 3zu}));
        // Right 1,1 to 1,3
        EXPECT_EQ(1zu, g.get_in_degree({layer::RIGHT, 1zu, 1zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::RIGHT, 1zu, 2zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::RIGHT, 1zu, 3zu}));
        // Right 2,1 to 2,3
        EXPECT_EQ(1zu, g.get_in_degree({layer::RIGHT, 2zu, 1zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::RIGHT, 2zu, 2zu}));
        EXPECT_EQ(2zu, g.get_in_degree({layer::RIGHT, 2zu, 3zu}));
    }

    TEST(PairwiseExtendedGapAlignmentGraphTest, GetEdgeData) {
        std::string seq1 { "ac" };
        std::string seq2 { "abc" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        EXPECT_EQ(1.0f64, g.get_edge_data({{layer::DIAGONAL, 0zu, 0zu}, {layer::DIAGONAL, 1zu, 1zu}}));
        EXPECT_EQ(-1.0f64, g.get_edge_data({{layer::DIAGONAL, 0zu, 1zu}, {layer::DIAGONAL, 1zu, 2zu}}));
        EXPECT_EQ(0.0f64, g.get_edge_data({{layer::DIAGONAL, 0zu, 0zu}, {layer::DOWN, 1zu, 0zu}}));  // from match to extended gap
        EXPECT_EQ(0.0f64, g.get_edge_data({{layer::DIAGONAL, 0zu, 0zu}, {layer::RIGHT, 0zu, 1zu}}));  // from match to extended gap
        EXPECT_EQ(0.0f64, g.get_edge_data({{layer::RIGHT, 0zu, 1zu}, {layer::DIAGONAL, 0zu, 1zu}}));  // from extended gap to match
        EXPECT_EQ(0.0f64, g.get_edge_data({{layer::DOWN, 1zu, 0zu}, {layer::DIAGONAL, 1zu, 0zu}}));  // from extended gap to match
        EXPECT_EQ(0.1f64, g.get_edge_data({{layer::DOWN, 1zu, 0zu}, {layer::DOWN, 2zu, 0zu}}));  // from extended gap to extended gap
        EXPECT_EQ(0.1f64, g.get_edge_data({{layer::RIGHT, 0zu, 1zu}, {layer::RIGHT, 0zu, 2zu}}));  // from extended gap to extended gap
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
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        using G = std::decay_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        EXPECT_EQ(G::limits(g.grid_down_cnt, g.grid_right_cnt).max_slice_nodes_cnt, 7zu);
        EXPECT_EQ(g.slice_first_node(0zu), (N { layer::DIAGONAL, 0zu, 0zu }));
        EXPECT_EQ(g.slice_last_node(0zu), (N { layer::DOWN, 0zu, 2zu }));
        EXPECT_EQ(g.slice_first_node(1zu), (N { layer::DIAGONAL, 1zu, 0zu }));
        EXPECT_EQ(g.slice_last_node(1zu), (N { layer::DOWN, 1zu, 2zu }));

        EXPECT_EQ(G::limits(g.grid_down_cnt, g.grid_right_cnt).max_resident_nodes_cnt, 0zu);
        EXPECT_EQ(g.resident_nodes().size(), 0zu);

        EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 0zu, 0zu }), (N { layer::DOWN, 0zu, 1zu })); // grid_down=0
        EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 0zu, 1zu }), (N { layer::RIGHT, 0zu, 1zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 0zu, 1zu }), (N { layer::DIAGONAL, 0zu, 1zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 0zu, 1zu }), (N { layer::DOWN, 0zu, 2zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 0zu, 2zu }), (N { layer::RIGHT, 0zu, 2zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 0zu, 2zu }), (N { layer::DIAGONAL, 0zu, 2zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 1zu, 0zu }), (N { layer::DOWN, 1zu, 1zu })); // grid_down=1
        EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 1zu, 1zu }), (N { layer::RIGHT, 1zu, 1zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 1zu, 1zu }), (N { layer::DIAGONAL, 1zu, 1zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::DIAGONAL, 1zu, 1zu }), (N { layer::DOWN, 1zu, 2zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::DOWN, 1zu, 2zu }), (N { layer::RIGHT, 1zu, 2zu }));
        EXPECT_EQ(g.slice_next_node(N { layer::RIGHT, 1zu, 2zu }), (N { layer::DIAGONAL, 1zu, 2zu }));

        EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 1zu, 2zu }), (N { layer::RIGHT, 1zu, 2zu })); // grid_down=1
        EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 1zu, 2zu }), (N { layer::DOWN, 1zu, 2zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 1zu, 2zu }), (N { layer::DIAGONAL, 1zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 1zu, 1zu }), (N { layer::RIGHT, 1zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 1zu, 1zu }), (N { layer::DOWN, 1zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 1zu, 1zu }), (N { layer::DIAGONAL, 1zu, 0zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 0zu, 2zu }), (N { layer::RIGHT, 0zu, 2zu })); // grid_down=1
        EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 0zu, 2zu }), (N { layer::DOWN, 0zu, 2zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 0zu, 2zu }), (N { layer::DIAGONAL, 0zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DIAGONAL, 0zu, 1zu }), (N { layer::RIGHT, 0zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::RIGHT, 0zu, 1zu }), (N { layer::DOWN, 0zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { layer::DOWN, 0zu, 1zu }), (N { layer::DIAGONAL, 0zu, 0zu }));

        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 0zu, 0zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 0zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 0zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 0zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 0zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 0zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 1zu, 0zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 1zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 1zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 1zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DOWN, 1zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::RIGHT, 1zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.outputs_to_residents(N { layer::DIAGONAL, 1zu, 2zu })), (std::vector<E> {}));

        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 0zu, 0zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 0zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 0zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 0zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 0zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 0zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 1zu, 0zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 1zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 1zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 1zu, 1zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DOWN, 1zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::RIGHT, 1zu, 2zu })), (std::vector<E> {}));
        EXPECT_EQ(to_vector(g.inputs_from_residents(N { layer::DIAGONAL, 1zu, 2zu })), (std::vector<E> {}));
    }
}