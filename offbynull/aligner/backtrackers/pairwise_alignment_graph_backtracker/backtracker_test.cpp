#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/directed_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <iostream>
#include <format>
#include <cstddef>

namespace {
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    TEST(PairwiseAlignmentGraphBacktrackTest, FindMaxPathOnGridGraph) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };

        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using E = decltype(g)::E;

        backtracker<true, decltype(g)> _backtracker{};
        const auto& [path, weight] {
            _backtracker.find_max_path(g)
        };
        for (const E& e : path) {
            const auto& [n1, n2] { e };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(
            path,
            (std::vector<E> {
                { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} },
                { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} }
            })
        );
        EXPECT_EQ(weight, 1.0);
    }
}
