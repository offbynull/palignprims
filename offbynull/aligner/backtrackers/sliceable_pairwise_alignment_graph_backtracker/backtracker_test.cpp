#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <stdfloat>
#include <random>
#include <string>
#include <iostream>
#include <ostream>
#include <vector>
#include <format>
#include <cstddef>
#include <ranges>
#include <cmath>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    TEST(OABSBacktrackerTest, GlobalTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        std::string seq1 { "abc" };
        std::string seq2 { "azc" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<true, decltype(g)> backtracker_ {};
        const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
        for (const E& e : path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        std::vector<E> option1 {
            E { N { 0zu, 0zu }, N { 1zu, 1zu } },
            E { N { 1zu, 1zu }, N { 2zu, 1zu } },
            E { N { 2zu, 1zu }, N { 2zu, 2zu } },
            E { N { 2zu, 2zu }, N { 3zu, 3zu } }
        };
        std::vector<E> option2 {
            E { N { 0zu, 0zu }, N { 1zu, 1zu } },
            E { N { 1zu, 1zu }, N { 1zu, 2zu } },
            E { N { 1zu, 2zu }, N { 2zu, 2zu } },
            E { N { 2zu, 2zu }, N { 3zu, 3zu } }
        };
        EXPECT_TRUE(path == option1 || path == option2);
    }

    TEST(OABSBacktrackerTest, LocalTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlmnzzzzz" };
        pairwise_local_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<true, decltype(g)> backtracker_ {};
        const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
        for (const E& e : path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
        std::vector<E> expected_path {
            E { edge_type::FREE_RIDE, { N { 0zu, 0zu }, N { 5zu, 5zu } } },
            E { edge_type::NORMAL, { N { 5zu, 5zu }, N { 6zu, 6zu } } },
            E { edge_type::NORMAL, { N { 6zu, 6zu }, N { 7zu, 7zu } } },
            E { edge_type::NORMAL, { N { 7zu, 7zu }, N { 8zu, 8zu } } },
            E { edge_type::FREE_RIDE, { N { 8zu, 8zu }, N { 13zu, 13zu } } }
        };
        EXPECT_EQ(expected_path, path);
    }

    TEST(OABSBacktrackerTest, OverlapTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmn" };
        std::string seq2 { "lmnzzzzz" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<true, decltype(g)> backtracker_ {};
        const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
        for (const E& e : path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge_type;
        std::vector<E> expected_path {
            E { edge_type::FREE_RIDE, { N { 0zu, 0zu }, N { 5zu, 0zu } } },
            E { edge_type::NORMAL, { N { 5zu, 0zu }, N { 6zu, 1zu } } },
            E { edge_type::NORMAL, { N { 6zu, 1zu }, N { 7zu, 2zu } } },
            E { edge_type::NORMAL, { N { 7zu, 2zu }, N { 8zu, 3zu } } },
            E { edge_type::FREE_RIDE, { N { 8zu, 3zu }, N { 8zu, 8zu } } }
        };
        EXPECT_EQ(expected_path, path);
    }

    TEST(OABSBacktrackerTest, FittingTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "lmn" };
        pairwise_fitting_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<true, decltype(g)> backtracker_ {};
        const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
        for (const E& e : path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge_type;
        std::vector<E> expected_path {
            E { edge_type::FREE_RIDE, { N { 0zu, 0zu }, N { 5zu, 0zu } } },
            E { edge_type::NORMAL, { N { 5zu, 0zu }, N { 6zu, 1zu } } },
            E { edge_type::NORMAL, { N { 6zu, 1zu }, N { 7zu, 2zu } } },
            E { edge_type::NORMAL, { N { 7zu, 2zu }, N { 8zu, 3zu } } },
            E { edge_type::FREE_RIDE, { N { 8zu, 3zu }, N { 13zu, 3zu } } }
        };
        EXPECT_EQ(expected_path, path);
    }

    TEST(OABSBacktrackerTest, ExtendedGapTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-0.1f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaalaa" };
        std::string seq2 { "l" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };
        using E = typename decltype(g)::E;

        std::vector<E> expected_path {};
        std::float64_t expected_weight {};
        {
            offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker<
                true,
                decltype(g)
            > backtracker_ {};
            const auto& [path, weight] { backtracker_.find_max_path(g) };
            for (const E& e : path) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;
            std::cout << weight << std::endl;
            expected_path = path;
            expected_weight = weight;
        }

        std::vector<E> actual_path {};
        std::float64_t actual_weight {};
        {
            backtracker<true, decltype(g)> backtracker_ {};
            const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
            for (const E& e : path) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;
            std::cout << weight << std::endl;
            actual_path = path;
            actual_weight = weight;
        }

        EXPECT_NEAR(expected_weight, actual_weight, 0.001);
        EXPECT_EQ(expected_path, actual_path);
    }

    TEST(OABSBacktrackerTest, ExtendedGapTest2) {
        // This specific test case is from the randomized tests in the function below. It was failing and so it was moved here to help
        // debug. Keeping it here just in case the bug comes back
        auto substitution_scorer {
            simple_scorer<true, char, char, std::float64_t>::create_substitution(0.35204742999374439f64, 0.79108113322398843f64)
        };
        auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.93320748544184262f64) };
        auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(-0.17625114138457276f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(-0.55461590857866616f64) };
        std::string seq1 { "bj" };
        std::string seq2 { "ya" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };
        using E = typename decltype(g)::E;

        // N first_node { *g.slice_nodes(1u).begin() };
        // offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph<
        //     true,
        //     decltype(g)
        // > suffix_g { g, first_node };
        // for (const N& n : suffix_g.slice_nodes(0u)) {
        //     const auto& [n_layer, n_down, n_right] { n };
        //     std::cout << n_down << '/' << n_right << '/' << static_cast<int>(n_layer) << std::endl;
        // }
        // offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph<
        //     true,
        //     decltype(suffix_g)
        // > reversed_suffix_g { suffix_g };
        // offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker<
        //     true,
        //     decltype(reversed_suffix_g)
        // > backward_walker { reversed_suffix_g };
        // while (!backward_walker.next()) {}
        // auto ret { backward_walker.find(N { offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer::DOWN, 1u, 2u }) };

        std::vector<E> expected_path {};
        std::float64_t expected_weight {};
        {
            offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker<
                true,
                decltype(g)
            > backtracker_ {};
            const auto& [path, weight] { backtracker_.find_max_path(g) };
            for (const E& e : path) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;
            std::cout << weight << std::endl;
            expected_path = path;
            expected_weight = weight;
            std::float64_t weight_summed {};
            for (const E& e : path) {
                weight_summed += g.get_edge_data(e);
            }
            std::cout << "normal real weight from edges: " << weight_summed << std::endl;
        }

        std::vector<E> actual_path {};
        std::float64_t actual_weight {};
        {
            backtracker<true, decltype(g)> backtracker_ {};
            const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
            for (const E& e : path) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;
            std::cout << weight << std::endl;
            actual_path = path;
            actual_weight = weight;
            std::float64_t weight_summed {};
            for (const E& e : path) {
                weight_summed += g.get_edge_data(e);
            }
            std::cout << "sliced real weight from edges: " << weight_summed << std::endl;
        }

        EXPECT_NEAR(expected_weight, actual_weight, 0.001);
        EXPECT_EQ(expected_path, actual_path);
    }

    TEST(OABSBacktrackerTest, ExtendedGapTest3) {
        // This specific test case is from the randomized tests in the function below. It was failing and so it was moved here to help
        // debug. Keeping it here just in case the bug comes back
        auto substitution_scorer {
            simple_scorer<true, char, char, std::float64_t>::create_substitution(0.55939218269393787f64, -0.0048009351678480749f64)
        };
        auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.082629150808506191f64) };
        auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.71003697638657703f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(-0.68120966590317833f64) };
        std::string seq1 { "rczs" };
        std::string seq2 { "r" };
        pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };
        using E = typename decltype(g)::E;

        // using offbynull::aligner::graph::utils::pairwise_graph_to_graphviz;
        // N first_node { *g.slice_nodes(2u).begin() };
        // offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph<
        //     decltype(g)
        // > g_1 { g };
        // offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph<
        //     true,
        //     decltype(g_1)
        // > g_2 { g_1, first_node };
        // std::cout << pairwise_graph_to_graphviz(
        //     g_2,
        //     [&](auto n) {
        //         const auto& [layer_, down, right] { n };
        //         std::string layer_str {};
        //         switch (layer_) {
        //             case offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer::DOWN:
        //                 layer_str = "DOWN";
        //             break;
        //             case offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer::RIGHT:
        //                 layer_str = "RIGHT";
        //             break;
        //             case offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer::DIAGONAL:
        //                 layer_str = "DIAG";
        //             break;
        //         }
        //         return std::string { std::format(" {}x {}x {}", down, right, layer_str) };
        //     }
        // );

        std::vector<E> expected_path {};
        std::float64_t expected_weight {};
        {
            offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker<
                true,
                decltype(g)
            > backtracker_ {};
            const auto& [path, weight] { backtracker_.find_max_path(g) };
            for (const E& e : path) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;
            std::cout << weight << std::endl;
            expected_path = path;
            expected_weight = weight;
            std::float64_t weight_summed {};
            for (const E& e : path) {
                weight_summed += g.get_edge_data(e);
            }
            std::cout << "normal real weight from edges: " << weight_summed << std::endl;
        }

        std::vector<E> actual_path {};
        std::float64_t actual_weight {};
        {
            backtracker<true, decltype(g)> backtracker_ {};
            const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
            for (const E& e : path) {
                std::cout << e << ' ';
            }
            std::cout << std::endl;
            std::cout << weight << std::endl;
            actual_path = path;
            actual_weight = weight;
            std::float64_t weight_summed {};
            for (const E& e : path) {
                weight_summed += g.get_edge_data(e);
            }
            std::cout << "sliced real weight from edges: " << weight_summed << std::endl;
        }

        EXPECT_NEAR(expected_weight, actual_weight, 0.001);
        EXPECT_EQ(expected_path, actual_path);
    }

    TEST(OABSBacktrackerTest, RandomizedTesting) {
        std::mt19937_64 rand { 12345 };
        auto random_integer { [&](auto a, auto b) -> decltype(a) { return std::uniform_int_distribution<decltype(a)>(a,b)(rand); } };
        auto random_float { [&](auto a, auto b) -> decltype(a) { return std::uniform_real_distribution<decltype(a)>(a,b)(rand); } };
        auto random_string {
            [&](std::size_t max_len) {
                auto len { random_integer(0zu, max_len) };
                std::string ret {};
                for (auto _ : std::views::iota(0zu, random_integer(0zu, len))) {
                    ret += random_integer('a', 'z');
                }
                return ret;
            }
        };
        for ([[maybe_unused]] auto _ : std::views::iota(0u, 1000u)) {
            auto substitution_scorer {
                simple_scorer<true, char, char, std::float64_t>::create_substitution(
                    random_float(-1.0f64, 1.0f64),
                    random_float(-1.0f64, 1.0f64)
                )
            };
            auto initial_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(random_float(-1.0f64, 1.0f64)) };
            auto extended_gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(random_float(-1.0f64, 1.0f64)) };
            auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(random_float(-1.0f64, 1.0f64)) };
            std::string seq1 { random_string(5zu) };
            std::string seq2 { random_string(5zu) };
            pairwise_extended_gap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
                seq1,
                seq2,
                substitution_scorer,
                initial_gap_scorer,
                extended_gap_scorer,
                freeride_scorer
            };
            using E = typename decltype(g)::E;

            std::vector<E> expected_path {};
            std::float64_t expected_weight {};
            {
                offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker<
                    true,
                    decltype(g)
                > backtracker_ {};
                const auto& [path, weight] { backtracker_.find_max_path(g) };
                expected_path = path;
                // expected_weight = weight;  // don't use weight -- instead manually tally up the weight from the edges (just to be sure)
                for (const E& e : path) {
                    expected_weight += g.get_edge_data(e);
                }
            }

            std::vector<E> actual_path {};
            std::float64_t actual_weight {};
            {
                backtracker<true, decltype(g)> backtracker_ {};
                const auto& [path, weight] { backtracker_.find_max_path(g, 0.000001f64) };
                actual_path = path;
                // actual_weight = weight;  // don't use weight -- instead manually tally up the weight from the edges (just to be sure)
                for (const E& e : path) {
                    actual_weight += g.get_edge_data(e);
                }
            }

            if (std::abs(expected_weight - actual_weight) > 0.001) {
                std::cout << "err" << std::endl;
            }

            EXPECT_NEAR(expected_weight, actual_weight, 0.001);
            // EXPECT_EQ(expected_path, actual_path);  // if more than 1 optimal path, this test may fail
        }
    }
}
