#ifndef OFFBYNULL_ALIGNER_GRAPH_UTILS_H
#define OFFBYNULL_ALIGNER_GRAPH_UTILS_H

#include <cctype>
#include <format>
#include <string>
#include <concepts>
#include "pairwise_alignment_graph.h"
#include "offbynull/aligner/graph/graph.h"

/**
 * Utilities for @ref offbynull::aligner::graph::graph::readable_graph and its derivatives.
 */
namespace offbynull::aligner::graph::utils {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;

    /**
     * Escape an identifier for inclusion into GraphViz script.
     *
     * @param in Identifier.
     * @return \c in escaped for inclusion into dot script.
     */
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

    /**
     * Escape a string for inclusion into GraphViz script.
     *
     * @param in String.
     * @return \c in escaped for inclusion into dot script.
     */
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

    /**
     * Convert an @ref offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph to a GraphViz script. Each
     * node is converted to a string via \c n_encoder and placed into the GraphViz script at its position within \c g multiplied by
     * \c spacing and offset by \c depth*depth_offset. For example, given an invocation with \c scaling=10 and \c depth_offset=2, the node
     * at ...
     *
     *  * grid offset \code (1, 6) \endcode and depth of \c 0 would show up in the GraphViz script at \code (10*1+0*2, 10*6+0*2) \endcode .
     *  * grid offset \code (1, 6) \endcode and depth of \c 1 would show up in the GraphViz script at \code (10*1+1*2, 10*6+1*2) \endcode .
     *  * ...
     *
     * @tparam G Graph type.
     * @tparam N_ENCODER Type of function which encodes node IDs to a GraphViz identifier.
     * @param g Graph.
     * @param n_encoder Function used to convert a node ID to a GraphViz identifier.
     * @param spacing Scaling factor to apply to each node's position when inserting into GraphViz.
     * @param depth_offset Scaling factor to apply to each node's depth (after applying by \c spacing) when inserting into GraphViz.
     * @return GraphViz script.
     */
    template<readable_pairwise_alignment_graph G, typename N_ENCODER>
    requires requires(const N_ENCODER& n_encoder, const typename G::N& n) {
        { n_encoder(n) } -> std::convertible_to<std::string>;
    }
    std::string pairwise_graph_to_graphviz(
        const G &g,
        const N_ENCODER& n_encoder,
        const float spacing = 3.0f,
        const float depth_offset = 0.3f
    ) {
        using N = typename G::N;
        using E = typename G::E;
        std::string out {};
        out += "digraph G {\n";
        out += "  layout=fdp;\n";
        out += "  overlap=true;\n";
        for (const N& n : g.get_nodes()) {
            const auto& [down, right, depth] { g.node_to_grid_offset(n) };
            std::string name { n_encoder(n) };
            float draw_y { (static_cast<float>(down) * spacing) + (depth_offset * static_cast<float>(depth)) };
            float draw_x { (static_cast<float>(right) * spacing) + (depth_offset * static_cast<float>(depth)) };
            out += std::format(
                "  {} [ label=\"{}\" pos=\"{},{}!\"];\n",
                escape_identifier_for_graphviz(name),
                escape_string_for_graphviz(name),
                draw_x,
                -draw_y
            );
        }
        for (const E& e : g.get_edges()) {
            std::string n1_name { n_encoder(g.get_edge_from(e)) };
            std::string n2_name { n_encoder(g.get_edge_to(e)) };
            out += std::format(
                "  {}->{};\n",
                escape_identifier_for_graphviz(n1_name),
                escape_identifier_for_graphviz(n2_name)
            );
        }
        out += "}\n";
        return out;
    }

    /**
     * Convert an @ref offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph to a GraphViz script. Each
     * node is converted to a string via \c std::format and placed into the GraphViz script at its position within \c g multiplied by
     * \c spacing and offset by \c depth*depth_offset. For example, given an invocation with \c scaling=10 and \c depth_offset=2, the node
     * at ...
     *
     *  * grid offset \code (1, 6) \endcode and depth of \c 0 would show up in the GraphViz script at \code (10*1+0*2, 10*6+0*2) \endcode.
     *  * grid offset \code (1, 6) \endcode and depth of \c 1 would show up in the GraphViz script at \code (10*1+1*2, 10*6+1*2) \endcode.
     *  * ...
     *
     * @tparam G Graph type.
     * @param g Graph.
     * @param spacing Scaling factor to apply to each node's position when inserting into GraphViz.
     * @param depth_offset Scaling factor to apply to each node's depth (after applying by \c spacing) when inserting into GraphViz.
     * @return GraphViz script.
     */
    template<readable_pairwise_alignment_graph G>
    requires requires(const typename G::N& n) {
        { std::format("{}", n) } -> std::convertible_to<std::string>;
    }
    std::string pairwise_graph_to_graphviz(
        const G &g,
        const float spacing = 3.0f,
        const float depth_offset = 0.3f
    ) {
        using N = typename G::N;

        return pairwise_graph_to_graphviz(
            g,
            [&](const N& n) {
                // const auto& [down, right, depth] { g.node_to_grid_offset(n) };
                // return std::string { std::format(" {}x {}x {}", down, right, depth) };
                return std::format("{}", n);
            },
            spacing,
            depth_offset
        );
    }

    /**
     * Convert an @ref offbynull::aligner::graph::graph::readable_graph to a GraphViz script. Each node is converted to a
     * string via \c n_encoder and placed into the GraphViz script at whatever position the dot layout engine deems best.
     *
     * @tparam G Graph type.
     * @tparam N_ENCODER Type of function which encodes node IDs to a GraphViz identifier.
     * @param g Graph.
     * @return GraphViz script.
     */
    template<readable_graph G, typename N_ENCODER>
    requires requires(const N_ENCODER& n_encoder, const typename G::N& n) {
        { n_encoder(n) } -> std::convertible_to<std::string>;
    }
    std::string graph_to_graphviz(
        const G &g,
        const N_ENCODER& n_encoder
    ) {
        using N = typename G::N;
        using E = typename G::E;

        std::string out {};
        out += "digraph G {\n";
        out += "  layout=dot;\n";
        out += "  overlap=true;\n";
        for (const N& n : g.get_nodes()) {
            std::string name { n_encoder(n) };
            out += std::format(
                "  {} [ label=\"{}\"];\n",
                escape_identifier_for_graphviz(name),
                escape_string_for_graphviz(name)
            );
        }
        for (const E& e : g.get_edges()) {
            std::string n1_name { n_encoder(g.get_edge_from(e)) };
            std::string n2_name { n_encoder(g.get_edge_to(e)) };
            out += std::format(
                "  {}->{};\n",
                escape_identifier_for_graphviz(n1_name),
                escape_identifier_for_graphviz(n2_name)
            );
        }
        out += "}\n";
        return out;
    }

    /**
     * Convert an @ref offbynull::aligner::graph::graph::readable_graph to a GraphViz script. Each node is converted to a
     * string via \c std::format and placed into the GraphViz script at whatever position the dot layout engine deems best.
     *
     * @tparam G Graph type.
     * @param g Graph.
     * @return GraphViz script.
     */
    template<readable_graph G>
    std::string graph_to_graphviz(
        const G &g
    ) {
        using N = typename G::N;

        return graph_to_graphviz(
            g,
            [&](const N& n) {
                // const auto& [down, right, depth] { g.node_to_grid_offset(n) };
                // return std::string { std::format(" {}x {}x {}", down, right, depth) };
                return std::format("{}", n);
            }
        );
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_UTILS_H
