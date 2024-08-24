#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/directed_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <stdfloat>
#include <string>
#include <ostream>
#include <iostream>
#include <vector>

namespace {
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    TEST(OABPBacktrackerTest, FindMaxPathOnGridGraph) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };

        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<
            true,
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using E = decltype(g)::E;

        backtracker<true, decltype(g)> _backtracker {};
        const auto& [path, weight] {
            _backtracker.find_max_path(g)
        };
        for (const E& e : path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(
            path,
            (std::vector<E> {
                E { { 0zu, 0zu }, { 1zu, 1zu } },
                E { { 1zu, 1zu }, { 1zu, 2zu } }
            })
        );
        EXPECT_EQ(weight, 1.0);
    }
}
