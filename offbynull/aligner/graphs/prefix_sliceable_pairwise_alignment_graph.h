#ifndef OFFBYNULL_ALIGNER_GRAPHS_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <utility>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::aligner::graph::graph::full_input_output_range;

    /**
     * View into an existing
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph, where the view is
     * isolated to some portion starting from the root node and ending at some arbitrary node.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Backing graph type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G
    >
    class prefix_sliceable_pairwise_alignment_graph {
    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::INDEX */
        using INDEX = typename G::INDEX;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N */
        using N = typename G::N;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::E */
        using E = typename G::E;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ED */
        using ED = typename G::ED;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ND */
        using ND = typename G::ND;

    private:
        static constexpr INDEX I0 { static_cast<INDEX>(0zu) };
        static constexpr INDEX I1 { static_cast<INDEX>(1zu) };
        static constexpr INDEX I2 { static_cast<INDEX>(2zu) };

        const G& g;
        const N new_leaf_node;

        bool node_out_of_bound(const N& node) const {
            const auto& [down_offset, right_offset, _] { g.node_to_grid_offset(node) };
            if (down_offset >= grid_down_cnt || right_offset >= grid_right_cnt) {
                return true;
            } else if (down_offset == grid_down_cnt - I1 || right_offset == grid_right_cnt - I1) {
                // In the same grid position as leaf node (but potentially at different depths). Can this node reach the
                // leaf node? If yes, it's not out-of-bound.
                return !g.is_reachable(node, new_leaf_node);
            }
            return false;
        }

        bool edge_out_of_bound(const E& edge) const {
            return node_out_of_bound(g.get_edge_from(edge)) || node_out_of_bound(g.get_edge_to(edge));
        }

    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_down_cnt */
        const INDEX grid_down_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_right_cnt */
        const INDEX grid_right_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_depth_cnt */
        static constexpr INDEX grid_depth_cnt { G::grid_depth_cnt };
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes_capacity */
        const std::size_t resident_nodes_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::path_edge_capacity */
        const std::size_t path_edge_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_incoming_edge_capacity */
        const std::size_t node_incoming_edge_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_outgoing_edge_capacity */
        const std::size_t node_outgoing_edge_capacity;

        /**
         * Construct an
         * @ref offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph instance.
         *
         * @param g_ Backing graph.
         * @param new_leaf_node_ Node within `g_` to designate as the leaf node (nodes after `new_leaf_node_` are hidden).
         */
        prefix_sliceable_pairwise_alignment_graph(
            const G& g_,
            const N& new_leaf_node_
        )
        : g { g_ }
        , new_leaf_node { new_leaf_node_ }
        , grid_down_cnt { std::get<0zu>(g.node_to_grid_offset(new_leaf_node)) + I1 }
        , grid_right_cnt { std::get<1zu>(g.node_to_grid_offset(new_leaf_node)) + I1 }
        , resident_nodes_capacity { g.resident_nodes_capacity }
        , path_edge_capacity { g.path_edge_capacity }
        , node_incoming_edge_capacity { g.node_incoming_edge_capacity }
        , node_outgoing_edge_capacity { g.node_outgoing_edge_capacity } {
            if constexpr (debug_mode) {
                if (!g.has_node(new_leaf_node)) {
                    throw std::runtime_error { "Leaf node not found" };
                }
            }
        }

        // The first implementation of this was proxing the exact type. For example...
        //
        // using get_edge_to_ret_type = decltype(g.get_edge_to(std::declval<E>()));
        // get_edge_to_ret_type get_edge_to(const E &edge) {
        //     return g.get_edge_to(...);
        // }
        //
        // Decided not to do this. No pairwise graph type uses ND&/ED&/N&/E&, so just go ahead and return concrete
        // object.

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_node_data */
        ND get_node_data(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.get_node_data(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_data */
        ED get_edge_data(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return g.get_edge_data(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_from */
        N get_edge_from(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return g.get_edge_from(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_to */
        N get_edge_to(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return g.get_edge_to(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge */
        std::tuple<N, N, ED> get_edge(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return g.get_edge(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_nodes */
        bidirectional_range_of_non_cvref<N> auto get_root_nodes() const {
            return g.get_root_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_node */
        N get_root_node() const {
            return g.get_root_node();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_nodes */
        bidirectional_range_of_non_cvref<N> auto get_leaf_nodes() const {
            return std::views::single(get_leaf_node());
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_node */
        N get_leaf_node() const {
            return new_leaf_node;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_nodes */
        bidirectional_range_of_non_cvref<N> auto get_nodes() const {
            return g.get_nodes()
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edges */
        bidirectional_range_of_non_cvref<E> auto get_edges() const {
            return g.get_edges()
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_node */
        bool has_node(const N& n) const {
            return g.has_node(n) && !node_out_of_bound(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_edge */
        bool has_edge(const E& e) const {
            return g.has_edge(e) && !edge_out_of_bound(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs_full */
        full_input_output_range<N, E, ED> auto get_outputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.get_outputs_full(n)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<0zu>(vals)); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs_full */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.get_inputs_full(n)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<0zu>(vals)); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.get_outputs(n)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(vals); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.get_inputs(n)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(vals); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_outputs */
        bool has_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return get_out_degree(n) > I0;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_inputs */
        bool has_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return get_in_degree(n) > I0;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_out_degree */
        std::size_t get_out_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto outputs { std::views::common(get_outputs(n)) };
            auto dist { std::ranges::distance(outputs) };
            return static_cast<std::size_t>(dist);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_in_degree */
        std::size_t get_in_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto inputs { std::views::common(get_inputs(n)) };
            auto dist { std::ranges::distance(inputs) };
            return static_cast<std::size_t>(dist);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::edge_to_element_offsets */
        std::optional<std::pair<std::optional<INDEX>, std::optional<INDEX>>> edge_to_element_offsets(
            const E& e
        ) const {
            return g.edge_to_element_offsets(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_to_grid_offset */
        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offset(const N& n) const {
            return g.node_to_grid_offset(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_offset_to_nodes */
        bidirectional_range_of_non_cvref<N> auto grid_offset_to_nodes(INDEX grid_down, INDEX grid_right) const {
            if constexpr (debug_mode) {
                if (grid_down >= grid_down_cnt || grid_right >= grid_right_cnt) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            N root_node { g.get_root_node() };
            N leaf_node { g.get_leaf_node() };
            return g.grid_offset_to_nodes(grid_down, grid_right)
                | std::views::filter([this, root_node, leaf_node](const N& node) {
                    return g.is_reachable(root_node, node) && g.is_reachable(node, leaf_node);
                });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(INDEX grid_down) const {
            return row_nodes(grid_down, g.get_root_node(), new_leaf_node);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            if constexpr (debug_mode) {
                if (!has_node(root_node) || !has_node(leaf_node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.row_nodes(grid_down, root_node, leaf_node)
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::is_reachable */
        bool is_reachable(const N& n1, const N& n2) const {
            if constexpr (debug_mode) {
                if (!has_node(n1) || !has_node(n2)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.is_reachable(n1, n2);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes */
        bidirectional_range_of_non_cvref<N> auto resident_nodes() const {
            return g.resident_nodes()
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::outputs_to_residents */
        bidirectional_range_of_non_cvref<E> auto outputs_to_residents(const N& n) const {
            return g.outputs_to_residents(n)
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::inputs_from_residents */
        bidirectional_range_of_non_cvref<E> auto inputs_from_residents(const N& n) const {
            return g.inputs_from_residents(n)
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }
    };


    /**
     * Create @ref offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph
     * instance, where template parameters are deduced / inferred from arguments passed in.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @return New @ref offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph
     *     instance.
     */
    template<
        bool debug_mode
    >
    auto create_prefix_sliceable_pairwise_alignment_graph(
        const sliceable_pairwise_alignment_graph auto& g,
        const typename std::remove_cvref_t<decltype(g)>::N& new_leaf_node
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        return prefix_sliceable_pairwise_alignment_graph<debug_mode, G> { g, new_leaf_node };
    }
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
