#ifndef OFFBYNULL_ALIGNER_GRAPH_UTILS_H
#define OFFBYNULL_ALIGNER_GRAPH_UTILS_H

#include <concepts>
#include <iterator>
#include <format>
#include <string>
#include <ranges>

namespace offbynull::aligner::graph::utils {
    std::string graph_to_string(const auto &g) {
        std::string out {};
        for (const auto& node : g.get_nodes()) {
            out += std::format("node {}: {}\n", node, g.get_node_data(node));
            for (const auto& edge : g.get_outputs(node)) {
                auto [from_node, to_node, edge_data] = g.get_edge(edge);
                out += std::format("  edge {} pointing to node {}: {}\n", edge, to_node, edge_data);
            }
        }
        return out;
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_UTILS_H
