#include <format>
#include <string>
#include <iostream>
#include "offbynull/aligner/graphs/directed_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/aligner/graph/utils.h"
#include "gtest/gtest.h"

namespace {
    TEST(OAGUtilsTest, DirectGraphToGraphviz) {
        using offbynull::aligner::graph::utils::graph_to_graphviz;
        using offbynull::aligner::graphs::directed_graph::directed_graph;

        directed_graph<true, std::string, std::string, std::string, std::string> g {};

        g.insert_node("A", "");
        g.insert_node("B", "");
        g.insert_node("C", "");
        g.insert_node("D", "");
        g.insert_edge("AB1", "A", "B", "MY EDGE DATA FOR 1st AB");
        g.insert_edge("AB2", "A", "B", "MY EDGE DATA FOR 2nd AB");
        g.insert_edge("BC", "B", "C", "");
        g.insert_edge("CD", "C", "D", "");
        g.insert_edge("AD", "A", "D", "");
        g.insert_node("E", "");
        g.insert_edge("DE", "D", "E", "");
        g.delete_edge("AD");

        std::cout << graph_to_graphviz(g) << std::endl;
    }

    TEST(OAGUtilsTest, PairwiseGraphToGraphviz) {
        using offbynull::aligner::graph::utils::pairwise_graph_to_graphviz;
        using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
        using offbynull::aligner::scorers::simple_scorer::simple_scorer;

        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.1f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride() };

        std::string seq1 { "hello" };
        std::string seq2 { "mellow" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };

        std::cout << pairwise_graph_to_graphviz(g) << std::endl;
    }
}