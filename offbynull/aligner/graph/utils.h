#ifndef OFFBYNULL_ALIGNER_GRAPH_UTILS_H
#define OFFBYNULL_ALIGNER_GRAPH_UTILS_H

#include <cctype>
#include <format>
#include <string>
#include "pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/graph.h"

namespace offbynull::aligner::graph::utils {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;

    inline std::string escape_identifier_for_graphviz(const std::string& in) {
        std::string ret { "_" };
        for (const auto& ch : in) {
            if (std::isalnum(ch)) {
                ret += ch;
            } else {
                ret += "_";
                ret += std::to_string(static_cast<int>(ch));
            }
        }
        return ret;
    }

    inline std::string escape_string_for_graphviz(const std::string& in) {
        std::string ret {};
        for (const auto& ch : in) {
            if (ch == '"') {
                ret += "\\\"";
            } else if (ch == '\\') {
                ret += "\\\\";
            } else if (ch == '\r') {
                ret += "\\r";
            } else if (ch == '\n') {
                ret += "\\n";
            } else if (ch == '\t') {
                ret += "\\t";
            } else {
                ret += ch;
            }
        }
        return ret;
    }

    std::string pairwise_graph_to_graphviz(
        const readable_pairwise_alignment_graph auto &g,
        auto to_name_func,
        float depth_offset = 0.3f,
        float space_between_grid_offsets = 3.0f
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
            out += std::format(
                "  {} [ label=\"{}\" pos=\"{},{}!\"];\n",
                escape_identifier_for_graphviz(name),
                escape_string_for_graphviz(name),
                draw_x,
                -draw_y
            );
        }
        for (const auto& e : g.get_edges()) {
            std::string n1_name { to_name_func(g.get_edge_from(e)) };
            std::string n2_name { to_name_func(g.get_edge_to(e)) };
            out += std::format(
                "  {}->{};\n",
                escape_identifier_for_graphviz(n1_name),
                escape_identifier_for_graphviz(n2_name)
            );
        }
        out += "}\n";
        return out;
    }

    std::string pairwise_graph_to_graphviz(
        const readable_pairwise_alignment_graph auto &g,
        float depth_offset = 0.3f,
        float space_between_grid_offsets = 3.0f
    ) {
        return pairwise_graph_to_graphviz(
            g,
            [&](auto n) {
                // const auto& [down, right, depth] { g.node_to_grid_offsets(n) };
                // return std::string { std::format(" {}x {}x {}", down, right, depth) };
                return std::format("{}", n);
            },
            depth_offset,
            space_between_grid_offsets
        );
    }

    std::string graph_to_graphviz(
        const readable_graph auto &g,
        auto to_name_func
    ) {
        std::string out {};
        out += "digraph G {\n";
        out += "  layout=dot;\n";
        out += "  overlap=true;\n";
        for (const auto& n : g.get_nodes()) {
            std::string name { to_name_func(n) };
            out += std::format(
                "  {} [ label=\"{}\"];\n",
                escape_identifier_for_graphviz(name),
                escape_string_for_graphviz(name)
            );
        }
        for (const auto& e : g.get_edges()) {
            std::string n1_name { to_name_func(g.get_edge_from(e)) };
            std::string n2_name { to_name_func(g.get_edge_to(e)) };
            out += std::format(
                "  {}->{};\n",
                escape_identifier_for_graphviz(n1_name),
                escape_identifier_for_graphviz(n2_name)
            );
        }
        out += "}\n";
        return out;
    }

    std::string graph_to_graphviz(
        const readable_graph auto &g
    ) {
        return graph_to_graphviz(
            g,
            [&](auto n) {
                // const auto& [down, right, depth] { g.node_to_grid_offsets(n) };
                // return std::string { std::format(" {}x {}x {}", down, right, depth) };
                return std::format("{}", n);
            }
        );
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_UTILS_H
