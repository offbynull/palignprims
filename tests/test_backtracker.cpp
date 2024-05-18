#include "grid_graph.h"
#include "directed_graph.h"
#include "backtracker.h"
#include "gtest/gtest.h"
#include <iostream>
#include <format>

namespace {
    TEST(BacktrackText, FindMaxPathOnGridGraph) {
        using N = std::pair<unsigned int, unsigned int>;
        using E = std::pair<N, N>;
        using ND = std::tuple<std::optional<double>, std::optional<E>>;
        using ED = double;

        auto g { offbynull::grid_graph::grid_graph::create_vector<ND, ED>(2u, 3u) };
        g.update_edge_data({ {0u, 0u}, {0u, 1u} }, -1.0); // this updates ALL indel edges
        g.update_edge_data({ {0u, 0u}, {1u, 1u} }, 3.0);
        auto [path, weight] = *find_max_path(
            g,
            g.get_root_node(),
            *g.get_leaf_nodes().begin(),
            [&g](N node, std::optional<double> weight, std::optional<E> edge) { g.update_node_data(node, { weight, edge }); },
            [&g](N node) { return g.get_node_data(node); },
            [&g](E edge) { return g.get_edge_data(edge); }
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

    TEST(BacktrackText, FindMaxPathOnDirectedGraph) {
        using N = std::pair<unsigned int, unsigned int>;
        using E = std::pair<N, N>;
        using ND = std::tuple<std::optional<double>, std::optional<E>>;
        using ED = double;
        using G = directed_graph<N, ND, E, ED>;

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
        auto [path, weight] = *find_max_path(
            g,
            g.get_root_node(),
            *g.get_leaf_nodes().begin(),
            [&g](N node, std::optional<double> weight, std::optional<E> edge) { g.update_node_data(node, { weight, edge }); },
            [&g](N node) { return g.get_node_data(node); },
            [&g](E edge) { return g.get_edge_data(edge); }
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