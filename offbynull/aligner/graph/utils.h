#ifndef OFFBYNULL_ALIGNER_GRAPH_UTILS_H
#define OFFBYNULL_ALIGNER_GRAPH_UTILS_H

#include <format>
#include <string>

#include "pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/graph.h"

namespace offbynull::aligner::graph::utils {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;

    std::string graph_to_string(readable_graph auto &g) {
        std::string out {};
        for (const auto& node : g.get_nodes()) {
            out += std::format("node {}: {}\n", node, g.get_node_data(node));
            for (const auto& edge : g.get_outputs(node)) {
                const auto& [from_node, to_node, edge_data] { g.get_edge(edge) };
                out += std::format("  edge {} pointing to node {}: {}\n", edge, to_node, edge_data);
            }
        }
        return out;
    }


    std::string pairwise_graph_to_graphviz(
        readable_pairwise_alignment_graph auto &g,
        auto to_name_func,
        float depth_offset=0.3f,
        float space_between_grid_offsets=3.0f
    ) {
        std::string out {};
        out += "digraph G {\n";
        out += "  layout=fdp;\n";
        out += "  overlap=true;\n";
        for (const auto& n : g.get_nodes()) {
            const auto& [down, right, depth] { g.node_to_grid_offsets(n) };
            std::string name { to_name_func(n) };
            float draw_y { (down * space_between_grid_offsets) + (depth_offset * depth) };
            float draw_x { (right * space_between_grid_offsets) + (depth_offset * depth) };
            out += std::format("  _{} [ label=\"{}\" pos=\"{},{}!\"];\n", name, name, draw_x, -draw_y);
        }
        for (const auto& e : g.get_edges()) {
            std::string n1_name { to_name_func(g.get_edge_from(e)) };
            std::string n2_name { to_name_func(g.get_edge_to(e)) };
            out += std::format("  _{}->_{};\n", n1_name, n2_name);
        }
        out += "}\n";
        return out;
    }

    std::string pairwise_graph_to_dot(
        readable_pairwise_alignment_graph auto &g,
        float depth_offset=0.3f,
        float space_between_grid_offsets=3.0f
    ) {
        return pairwise_graph_to_dot(
            g,
            [&](auto n) {
                const auto& [down, right, depth] { g.node_to_grid_offsets(n) };
                return std::string { std::format("{}x{}x{}", down, right, depth) };
            },
            depth_offset,
            space_between_grid_offsets
        );
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_UTILS_H
