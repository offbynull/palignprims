#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <stdfloat>
#include <optional>
#include <tuple>
#include <string>
#include <iostream>
#include <ostream>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto walk_to_node(auto bidi_walker_, auto node) {
        using E = decltype(bidi_walker_)::E;
        using ED = decltype(bidi_walker_)::ED;

        auto result { bidi_walker_.find(node) };
        ED weight { result.forward_slot.backtracking_weight + result.backward_slot.backtracking_weight };
        std::optional<E> forward_edge { result.forward_slot.backtracking_edge };
        std::optional<E> backward_edge { result.backward_slot.backtracking_edge };
        return std::tuple<ED, std::optional<E>, std::optional<E>> { weight, forward_edge, backward_edge };
    }

    TEST(OABSBidiWalkerTest, WalkGlobal) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        std::string seq1 { "abcdefg" };
        std::string seq2 { "abcZefg" };
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

        using N = decltype(g)::N;

        // walk
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, down_offset) };
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    walk_to_node(bidi_walker_, N { down_offset, right_offset })
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { *forward_walk_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { *backward_walk_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }

        bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, 3u) };
        const auto& [weight, forward_walk_edge, backward_walk_edge] { walk_to_node(bidi_walker_,  N { 3u, 3u }) };
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(weight, 6u);
    }

    TEST(OABSBidiWalkerTest, WalkLocal) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlVnzzzzz" };
        pairwise_local_alignment_graph<
            true,
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = decltype(g)::N;

        // walk
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, down_offset) };
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    walk_to_node(bidi_walker_, N { down_offset, right_offset })
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }
        // const auto& weight { walk_to_node(bidi_walker_, N { 3u, 3u }) };
        // std::cout << std::endl;
        // std::cout << weight << std::endl;
        // EXPECT_EQ(weight, 6u);
    }

    TEST(OABSBidiWalkerTest, WalkFitting) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaalmnaaa" };
        std::string seq2 { "lmn" };
        pairwise_fitting_alignment_graph<
            true,
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = decltype(g)::N;

        // walk
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, down_offset) };
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    walk_to_node(bidi_walker_, N { down_offset, right_offset })
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }

        // test
        bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, g.grid_down_cnt - 1u) };
        auto final_weight { std::get<0>(walk_to_node(bidi_walker_, g.get_leaf_node())) };
        std::cout << "final weight: " << final_weight << std::endl;
        for (const auto& node : g.resident_nodes()) {
            const auto& [down_offset, right_offset] { node };
            const auto& [weight, forward_walk_edge, backward_walk_edge] {
                walk_to_node(bidi_walker_, N { down_offset, right_offset })
            };
            std::cout << "resident node: " << down_offset << ',' << right_offset << '=' << weight;
            if (forward_walk_edge.has_value()) {
                const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
            } else {
                std::cout << '(' << "fh_from_,_" << ')';
            }
            if (backward_walk_edge.has_value()) {
                const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
            } else {
                std::cout << '(' << "bh_to_,_" << ')';
            }
            std::cout << std::endl;
        }
    }

    TEST(OABSBidiWalkerTest, WalkOverlap) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmn" };
        std::string seq2 { "lmnzzzzz" };
        pairwise_overlap_alignment_graph<
            true,
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = decltype(g)::N;

        // walk
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, down_offset) };
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    walk_to_node(bidi_walker_, N { down_offset, right_offset })
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }

        // test
        bidi_walker<true, decltype(g)> bidi_walker_ { bidi_walker<true, decltype(g)>::create_and_initialize(g, g.grid_down_cnt - 1u) };
        auto final_weight { std::get<0>(walk_to_node(bidi_walker_, g.get_leaf_node())) };
        std::cout << "final weight: " << final_weight << std::endl;
        for (const auto& node : g.resident_nodes()) {
            const auto& [down_offset, right_offset] { node };
            const auto& [weight, forward_walk_edge, backward_walk_edge] {
                walk_to_node(bidi_walker_, N { down_offset, right_offset })
            };
            std::cout << "resident node: " << down_offset << ',' << right_offset << '=' << weight;
            if (forward_walk_edge.has_value()) {
                const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
            } else {
                std::cout << '(' << "fh_from_,_" << ')';
            }
            if (backward_walk_edge.has_value()) {
                const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
            } else {
                std::cout << '(' << "bh_to_,_" << ')';
            }
            std::cout << std::endl;
        }
    }

}
