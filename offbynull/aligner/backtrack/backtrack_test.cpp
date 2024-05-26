#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/graphs/directed_graph.h"
#include "offbynull/aligner/backtrack/backtrack.h"
#include "gtest/gtest.h"
#include <iostream>
#include <format>
#include <cstddef>
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"

namespace {
    using offbynull::aligner::backtrack::backtrack::backtracker;

    template<typename ND_, typename ED_, typename INDEX = unsigned int, bool error_check = true>
    auto create_vector(INDEX down_cnt, INDEX right_cnt) {
        return offbynull::aligner::graphs::grid_graph::grid_graph<
            ND_,
            ED_,
            INDEX,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ND_, INDEX>,
            offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator<ED_, INDEX>,
            error_check
        > {
            down_cnt,
            right_cnt
        };
    }

    TEST(BacktrackTest, FindMaxPathOnGridGraph) {
        using N = std::pair<unsigned int, unsigned int>;
        using E = std::pair<N, N>;
        using ND = std::tuple<std::optional<std::float64_t>, std::optional<E>>;
        using ED = std::float64_t;

        auto g { create_vector<ND, ED>(2u, 3u) };
        g.update_edge_data({ {0u, 0u}, {0u, 1u} }, -1.0); // this updates ALL indel edges
        g.update_edge_data({ {0u, 0u}, {1u, 1u} }, 3.0);
        backtracker<decltype(g), std::uint8_t, ED> _backtracker{};
        auto [path, weight] = _backtracker.find_max_path(
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        );
        for (const E& e : path) {
            const auto& [n1, n2] = e;
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(
            path,
            (std::vector<E> {
                { std::pair{0u, 0u}, std::pair{1u, 1u} },
                { std::pair{1u, 1u}, std::pair{1u, 2u} }
            })
        );
        EXPECT_EQ(weight, 2.0);
    }

    TEST(BacktrackTest, FindMaxPathOnDirectedGraph) {
        using N = std::pair<unsigned int, unsigned int>;
        using E = std::pair<N, N>;
        using ND = std::tuple<std::optional<std::float64_t>, std::optional<E>>;
        using ED = std::float64_t;
        using G = offbynull::aligner::graphs::directed_graph::directed_graph<N, ND, E, ED>;

        G g {};
        g.insert_node(std::pair{0u, 0u}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{0u, 1u}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{0u, 2u}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{1u, 0u}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{1u, 1u}, {std::nullopt, std::nullopt});
        g.insert_node(std::pair{1u, 2u}, {std::nullopt, std::nullopt});
        g.insert_edge(std::pair { std::pair{0u, 0u}, std::pair{0u, 1u} }, std::pair{0u, 0u}, std::pair{0u, 1u}, 0.0);
        g.insert_edge(std::pair { std::pair{0u, 1u}, std::pair{0u, 2u} }, std::pair{0u, 1u}, std::pair{0u, 2u}, 0.0);
        g.insert_edge(std::pair { std::pair{1u, 0u}, std::pair{1u, 1u} }, std::pair{1u, 0u}, std::pair{1u, 1u}, 0.0);
        g.insert_edge(std::pair { std::pair{1u, 1u}, std::pair{1u, 2u} }, std::pair{1u, 1u}, std::pair{1u, 2u}, 0.0);
        g.insert_edge(std::pair { std::pair{0u, 0u}, std::pair{1u, 0u} }, std::pair{0u, 0u}, std::pair{1u, 0u}, 0.0);
        g.insert_edge(std::pair { std::pair{0u, 1u}, std::pair{1u, 1u} }, std::pair{0u, 1u}, std::pair{1u, 1u}, 0.0);
        g.insert_edge(std::pair { std::pair{0u, 2u}, std::pair{1u, 2u} }, std::pair{0u, 2u}, std::pair{1u, 2u}, 0.0);
        g.insert_edge(std::pair { std::pair{0u, 0u}, std::pair{1u, 1u} }, std::pair{0u, 0u}, std::pair{1u, 1u}, 0.0);
        g.insert_edge(std::pair { std::pair{0u, 1u}, std::pair{1u, 2u} }, std::pair{0u, 1u}, std::pair{1u, 2u}, 0.0);
        g.update_edge_data({ {0u, 0u}, {0u, 1u} }, 1.1);
        g.update_edge_data({ {1u, 1u}, {1u, 2u} }, 1.4);
        backtracker<decltype(g), std::size_t, ED> _backtracker{};
        auto [path, weight] = _backtracker.find_max_path(
            g,
            [&g](const E& edge) { return g.get_edge_data(edge); }
        );
        for (const E& e : path) {
            const auto& [n1, n2] = e;
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(
            path,
            (std::vector<E> {
                { std::pair{0u, 0u}, std::pair{0u, 1u} },
                { std::pair{0u, 1u}, std::pair{1u, 1u} },
                { std::pair{1u, 1u}, std::pair{1u, 2u} }
            })
        );
        EXPECT_EQ(weight, 2.5);
    }
}
