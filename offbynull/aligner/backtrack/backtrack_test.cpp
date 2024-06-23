#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/directed_graph.h"
#include "offbynull/aligner/backtrack/backtrack.h"
#include "gtest/gtest.h"
#include <iostream>
#include <format>
#include <cstddef>

namespace {
    using offbynull::aligner::backtrack::backtrack::backtracker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;

    TEST(BacktrackTest, FindMaxPathOnGridGraph) {
        auto match_lookup {
            [](
                const auto& edge,
                const char& down_elem,
                const char& right_elem
            ) -> std::float64_t {
                if (down_elem == right_elem) {
                    return 1.0f64;
                } else {
                    return -1.0f64;
                }
            }
        };
        auto indel_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };

        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup
        };

        using E = decltype(g)::E;
        using ED = decltype(g)::ED;

        backtracker<decltype(g), std::uint8_t, ED> _backtracker{};
        const auto& [path, weight] {
            _backtracker.find_max_path(
                g,
                [&g](const E& edge) { return g.get_edge_data(edge); }
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
                { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} },
                { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} }
            })
        );
        EXPECT_EQ(weight, 1.0);
    }

    TEST(BacktrackTest, FindMaxPathOnDirectedGraph) {
        using N = std::pair<unsigned int, unsigned int>;
        using E = std::pair<N, N>;
        using ND = std::tuple<std::optional<std::float64_t>, std::optional<E>>;
        using ED = std::float64_t;
        using G = offbynull::aligner::graphs::directed_graph::directed_graph<N, ND, E, ED>;

        G g {};
        g.insert_node(std::pair{0zu, 0zu}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{0zu, 1zu}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{0zu, 2zu}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{1zu, 0zu}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{1zu, 1zu}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{1zu, 2zu}, {std::nullopt, std::nullopt});
        g.insert_edge(std::pair { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} }, std::pair{0zu, 0zu}, std::pair{0zu, 1zu}, 0.0);
        g.insert_edge(std::pair { std::pair{0zu, 1zu}, std::pair{0zu, 2zu} }, std::pair{0zu, 1zu}, std::pair{0zu, 2zu}, 0.0);
        g.insert_edge(std::pair { std::pair{1zu, 0zu}, std::pair{1zu, 1zu} }, std::pair{1zu, 0zu}, std::pair{1zu, 1zu}, 0.0);
        g.insert_edge(std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} }, std::pair{1zu, 1zu}, std::pair{1zu, 2zu}, 0.0);
        g.insert_edge(std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 0zu} }, std::pair{0zu, 0zu}, std::pair{1zu, 0zu}, 0.0);
        g.insert_edge(std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 1zu} }, std::pair{0zu, 1zu}, std::pair{1zu, 1zu}, 0.0);
        g.insert_edge(std::pair { std::pair{0zu, 2zu}, std::pair{1zu, 2zu} }, std::pair{0zu, 2zu}, std::pair{1zu, 2zu}, 0.0);
        g.insert_edge(std::pair { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} }, std::pair{0zu, 0zu}, std::pair{1zu, 1zu}, 0.0);
        g.insert_edge(std::pair { std::pair{0zu, 1zu}, std::pair{1zu, 2zu} }, std::pair{0zu, 1zu}, std::pair{1zu, 2zu}, 0.0);
        g.update_edge_data({ {0zu, 0zu}, {0zu, 1zu} }, 1.1);
        g.update_edge_data({ {1zu, 1zu}, {1zu, 2zu} }, 1.4);
        backtracker<decltype(g), std::size_t, ED> _backtracker{};
        const auto& [path, weight] {
            _backtracker.find_max_path(
                g,
                [&g](const E& edge) { return g.get_edge_data(edge); }
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
                { std::pair{0zu, 0zu}, std::pair{0zu, 1zu} },
                { std::pair{0zu, 1zu}, std::pair{1zu, 1zu} },
                { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} }
            })
        );
        EXPECT_EQ(weight, 2.5);
    }
}
