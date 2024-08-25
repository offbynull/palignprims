#ifndef OFFBYNULL_ALIGNER_GRAPHS_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <utility>
#include <tuple>
#include <optional>
#include <ranges>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G
    >
    class reversed_sliceable_pairwise_alignment_graph {
    public:
        using INDEX = typename G::INDEX;
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using ND = typename G::ND;

    private:
        const G& g;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;
        const INDEX grid_depth_cnt;
        const std::size_t resident_nodes_capacity;
        const std::size_t path_edge_capacity;

        reversed_sliceable_pairwise_alignment_graph(const G& g_)
        : g { g_ }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt }
        , grid_depth_cnt { g.grid_depth_cnt }
        , resident_nodes_capacity { g.resident_nodes_capacity }
        , path_edge_capacity { g.path_edge_capacity } {}

        // The first implementation of this was proxing the exact type. For example...
        //
        // using get_edge_to_ret_type = decltype(g.get_edge_to(std::declval<E>()));
        // get_edge_to_ret_type get_edge_to(const E &edge) {
        //     return g.get_edge_to(...);
        // }
        //
        // Decided not to do this. No pairwise graph type uses ND&/ED&/N&/E&, so just go ahead and return concrete
        // object.

        ND get_node_data(const N& node) const {
            return g.get_node_data(node);
        }

        ED get_edge_data(const E& edge) const {
            return g.get_edge_data(edge);
        }

        N get_edge_from(const E& edge) const {
            return g.get_edge_to(edge);
        }

        N get_edge_to(const E& edge) const {
            return g.get_edge_from(edge);
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            return g.get_edge(edge);
        }

        auto get_root_nodes() const {
            return g.get_leaf_nodes();
        }

        N get_root_node() const {
            return g.get_leaf_node();
        }

        auto get_leaf_nodes() const {
            return g.get_root_nodes();
        }

        N get_leaf_node() const {
            return g.get_root_node();
        }

        auto get_nodes() const {
            return g.get_nodes();
        }

        auto get_edges() const {
            return g.get_edges();
        }

        bool has_node(const N& node) const {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) const {
            return g.has_edge(edge);
        }

        auto get_outputs_full(const N& node) const {
            return g.get_inputs_full(node);
        }

        auto get_inputs_full(const N& node) const {
            return g.get_outputs_full(node);
        }

        auto get_outputs(const N& node) const {
            return g.get_inputs(node);
        }

        auto get_inputs(const N& node) const {
            return g.get_outputs(node);
        }

        bool has_outputs(const N& node) const {
            return g.has_inputs(node);
        }

        bool has_inputs(const N& node) const {
            return g.has_outputs(node);
        }

        std::size_t get_out_degree(const N& node) const {
            return g.get_in_degree(node);
        }

        std::size_t get_in_degree(const N& node) const {
            return g.get_out_degree(node);
        }

        std::optional<std::pair<std::optional<INDEX>, std::optional<INDEX>>> edge_to_element_offsets(
            const E& edge
        ) const {
            auto offset { g.edge_to_element_offsets(edge) };
            if (!offset.has_value()) {
                return std::nullopt;
            }
            auto [v_idx, w_idx] { *offset };
            if (v_idx.has_value()) {
                *v_idx = g.grid_down_cnt - *v_idx - 1u;
            }
            if (w_idx.has_value()) {
                *w_idx = g.grid_right_cnt - *w_idx - 1u;
            }
            return offset;
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            const auto& [grid_down, grid_right, depth] { g.node_to_grid_offsets(node) };
            return { grid_down_cnt - grid_down - 1u, grid_right_cnt - grid_right - 1u, depth };
        }

        auto slice_nodes(INDEX grid_down) const {
            return g.slice_nodes(grid_down_cnt - grid_down - 1u) | std::views::reverse;
        }

        auto slice_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            return g.slice_nodes(grid_down_cnt - grid_down - 1u, leaf_node, root_node) | std::views::reverse;
        }

        bool is_reachable(const N& n1, const N& n2) const {
            return g.is_reachable(n2, n1);
        }

        auto resident_nodes() const {
            return g.resident_nodes();
        }

        auto outputs_to_residents(const N& node) const {
            return g.inputs_from_residents(node);
        }

        auto inputs_from_residents(const N& node) const {
            return g.outputs_to_residents(node);
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
