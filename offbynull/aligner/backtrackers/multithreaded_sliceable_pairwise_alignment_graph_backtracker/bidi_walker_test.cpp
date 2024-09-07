#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/multithreaded_sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/helpers/forkable_thread_pool.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <stdfloat>
#include <tuple>
#include <string>
#include <iostream>
#include <ostream>
#include <any>
#include <type_traits>
#include <random>

namespace {
    using offbynull::aligner::backtrackers::multithreaded_sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::helpers::forkable_thread_pool::forkable_thread_pool;
    using offbynull::utils::packable_optional;
    using offbynull::utils::random_printable_ascii;
    using offbynull::utils::is_debug_mode;

    auto walk_to_node(auto&& bidi_walker_, auto&& node) {
        using E = std::remove_cvref_t<decltype(bidi_walker_)>::E;
        using ED = std::remove_cvref_t<decltype(bidi_walker_)>::ED;

        auto result { bidi_walker_.find(node) };
        ED weight { result.forward_slot.backtracking_weight + result.backward_slot.backtracking_weight };
        packable_optional<E> forward_edge { result.forward_slot.backtracking_edge };
        packable_optional<E> backward_edge { result.backward_slot.backtracking_edge };
        return std::tuple<ED, packable_optional<E>, packable_optional<E>> { weight, forward_edge, backward_edge };
    }

    TEST(OABMBidiWalkerTest, LargeWalk) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        std::mt19937_64 rand { 12345 };
        std::string seq1 { random_printable_ascii(rand, 5000zu) };
        std::string seq2 { random_printable_ascii(rand, 5000zu) };
        pairwise_global_alignment_graph<
            is_debug_mode(),
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


        // walk
        forkable_thread_pool<is_debug_mode(), std::any> f_pool {};
        std::float64_t weight { bidi_walker<is_debug_mode(), decltype(g)>::converge_weight(f_pool, g, { 2500zu, 2500zu }) };
        std::cout << weight << std::endl;
    }

    TEST(OABMBidiWalkerTest, WalkGlobal) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(0.0f64) };
        std::string seq1 { "abcdefg" };
        std::string seq2 { "abcZefg" };
        pairwise_global_alignment_graph<
            is_debug_mode(),
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
        forkable_thread_pool<is_debug_mode(), std::any> f_pool {};
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
                bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, down_offset)
            };
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

        bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
            bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, 3u)
        };
        const auto& [weight, forward_walk_edge, backward_walk_edge] { walk_to_node(bidi_walker_,  N { 3u, 3u }) };
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(weight, 6u);
    }

    TEST(OABMBidiWalkerTest, WalkLocal) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlVnzzzzz" };
        pairwise_local_alignment_graph<
            is_debug_mode(),
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
        forkable_thread_pool<is_debug_mode(), std::any> f_pool {};
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
                bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, down_offset)
            };
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

    TEST(OABMBidiWalkerTest, WalkFitting) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaalmnaaa" };
        std::string seq2 { "lmn" };
        pairwise_fitting_alignment_graph<
            is_debug_mode(),
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
        forkable_thread_pool<is_debug_mode(), std::any> f_pool {};
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
                bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, down_offset)
            };
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
        bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
            bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, g.grid_down_cnt - 1u)
        };
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

    TEST(OABMBidiWalkerTest, WalkOverlap) {
        auto substitution_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmn" };
        std::string seq2 { "lmnzzzzz" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
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
        forkable_thread_pool<is_debug_mode(), std::any> f_pool {};
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
                bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, down_offset)
            };
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
        bidi_walker<is_debug_mode(), decltype(g)> bidi_walker_ {
            bidi_walker<is_debug_mode(), decltype(g)>::create_and_initialize(f_pool, g, g.grid_down_cnt - 1u)
        };
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
