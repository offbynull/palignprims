#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/directed_graph.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtracker.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <stdfloat>
#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <optional>
#include <ostream>
#include <iostream>

namespace {
    using offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::backtrackers::graph_backtracker::backtracker::heap_find_max_path;
    using offbynull::aligner::backtrackers::graph_backtracker::backtracker::stack_find_max_path;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_vector;

    TEST(OABGBacktrackerTest, FindMaxPathOnGridGraph) {
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
        using ED = decltype(g)::ED;

        auto edge_weight_accessor { [&g](const E& edge) { return g.get_edge_data(edge); } };
        backtracker<
            true,
            decltype(g),
            ED,
            decltype(edge_weight_accessor)
        > backtracker_ {};
        const auto& [path, weight] {
            backtracker_.find_max_path(
                g,
                edge_weight_accessor
            )
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

    TEST(OABGBacktrackerTest, FindMaxPathOnGridGraphViaHeapHelper) {
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

        const auto& [path, weight] {
            heap_find_max_path<true>(
                g,
                [&g](const E& edge) { return g.get_edge_data(edge); }
            )
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

    TEST(OABGBacktrackerTest, FindMaxPathOnGridGraphViaStackHelper) {
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

        const auto& [path, weight] {
            stack_find_max_path<true>(
                g,
                [&g](const E& edge) { return g.get_edge_data(edge); }
            )
        };

        for (const E& e : path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(
            copy_to_vector(path),
            (std::vector<E> {
                E { { 0zu, 0zu }, { 1zu, 1zu } },
                E { { 1zu, 1zu }, { 1zu, 2zu } }
            })
        );
        EXPECT_EQ(weight, 1.0);
    }

    TEST(OABGBacktrackerTest, FindMaxPathOnDirectedGraph) {
        using N = std::pair<unsigned int, unsigned int>;
        using E = std::pair<N, N>;
        using ND = std::tuple<std::optional<std::float64_t>, std::optional<E>>;
        using ED = std::float64_t;
        using G = offbynull::aligner::graphs::directed_graph::directed_graph<true, N, ND, E, ED>;

        G g {};
        g.insert_node(std::pair { 0zu, 0zu }, { std::nullopt, std::nullopt });
        g.insert_node(std::pair { 0zu, 1zu }, { std::nullopt, std::nullopt });
        g.insert_node(std::pair { 0zu, 2zu }, { std::nullopt, std::nullopt });
        g.insert_node(std::pair { 1zu, 0zu }, { std::nullopt, std::nullopt });
        g.insert_node(std::pair { 1zu, 1zu }, { std::nullopt, std::nullopt });
        g.insert_node(std::pair { 1zu, 2zu }, { std::nullopt, std::nullopt });
        g.insert_edge(std::pair { std::pair { 0zu, 0zu }, std::pair { 0zu, 1zu } }, std::pair { 0zu, 0zu }, std::pair { 0zu, 1zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 0zu, 1zu }, std::pair { 0zu, 2zu } }, std::pair { 0zu, 1zu }, std::pair { 0zu, 2zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 1zu, 0zu }, std::pair { 1zu, 1zu } }, std::pair { 1zu, 0zu }, std::pair { 1zu, 1zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 1zu, 1zu }, std::pair { 1zu, 2zu } }, std::pair { 1zu, 1zu }, std::pair { 1zu, 2zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 0zu, 0zu }, std::pair { 1zu, 0zu } }, std::pair { 0zu, 0zu }, std::pair { 1zu, 0zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 0zu, 1zu }, std::pair { 1zu, 1zu } }, std::pair { 0zu, 1zu }, std::pair { 1zu, 1zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 0zu, 2zu }, std::pair { 1zu, 2zu } }, std::pair { 0zu, 2zu }, std::pair { 1zu, 2zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 0zu, 0zu }, std::pair { 1zu, 1zu } }, std::pair { 0zu, 0zu }, std::pair { 1zu, 1zu }, 0.0);
        g.insert_edge(std::pair { std::pair { 0zu, 1zu }, std::pair { 1zu, 2zu } }, std::pair { 0zu, 1zu }, std::pair { 1zu, 2zu }, 0.0);
        g.update_edge_data({ { 0zu, 0zu }, { 0zu, 1zu } }, 1.1);
        g.update_edge_data({ { 1zu, 1zu }, { 1zu, 2zu } }, 1.4);

        auto edge_weight_accessor { [&g](const E& edge) { return g.get_edge_data(edge); } };
        backtracker<
            true,
            decltype(g),
            ED,
            decltype(edge_weight_accessor)
        > backtracker_ {};
        const auto& [path, weight] {
            backtracker_.find_max_path(
                g,
                edge_weight_accessor
            )
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
                { std::pair { 0zu, 0zu }, std::pair { 0zu, 1zu } },
                { std::pair { 0zu, 1zu }, std::pair { 1zu, 1zu } },
                { std::pair { 1zu, 1zu }, std::pair { 1zu, 2zu } }
            })
        );
        EXPECT_EQ(weight, 2.5);
    }
}
