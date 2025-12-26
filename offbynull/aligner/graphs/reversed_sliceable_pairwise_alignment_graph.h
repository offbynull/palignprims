#ifndef OFFBYNULL_ALIGNER_GRAPHS_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <utility>
#include <tuple>
#include <optional>
#include <ranges>
#include <type_traits>
#include <stdexcept>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::aligner::graph::graph::full_input_output_range;

    /**
     * View into an existing
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph, where the view
     * reverses all edges (root node becomes leaf node and vice versa).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Backing graph type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G
    >
    class reversed_sliceable_pairwise_alignment_graph {
    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N_INDEX */
        using N_INDEX = typename G::N_INDEX;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N */
        using N = typename G::N;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::E */
        using E = typename G::E;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ED */
        using ED = typename G::ED;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ND */
        using ND = typename G::ND;

    private:
        static constexpr N_INDEX I0 { static_cast<N_INDEX>(0zu) };
        static constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu) };
        static constexpr N_INDEX I2 { static_cast<N_INDEX>(2zu) };

        const G& g;

    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_down_cnt */
        const N_INDEX grid_down_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_right_cnt */
        const N_INDEX grid_right_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_depth_cnt */
        static constexpr N_INDEX grid_depth_cnt { G::grid_depth_cnt };
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
         * @ref offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph
         * instance.
         *
         * @param g_ Backing graph.
         */
        reversed_sliceable_pairwise_alignment_graph(const G& g_)
        : g { g_ }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt }
        , resident_nodes_capacity { g.resident_nodes_capacity }
        , path_edge_capacity { g.path_edge_capacity }
        , node_incoming_edge_capacity { g.node_outgoing_edge_capacity }
        , node_outgoing_edge_capacity { g.node_incoming_edge_capacity } {}

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
            return g.get_node_data(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_data */
        ED get_edge_data(const E& e) const {
            return g.get_edge_data(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_from */
        N get_edge_from(const E& e) const {
            return g.get_edge_to(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_to */
        N get_edge_to(const E& e) const {
            return g.get_edge_from(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge */
        std::tuple<N, N, ED> get_edge(const E& e) const {
            return g.get_edge(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_nodes */
        bidirectional_range_of_non_cvref<N> auto get_root_nodes() const {
            return g.get_leaf_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_node */
        N get_root_node() const {
            return g.get_leaf_node();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_nodes */
        bidirectional_range_of_non_cvref<N> auto get_leaf_nodes() const {
            return g.get_root_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_node */
        N get_leaf_node() const {
            return g.get_root_node();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_nodes */
        bidirectional_range_of_non_cvref<N> auto get_nodes() const {
            return g.get_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edges */
        bidirectional_range_of_non_cvref<E> auto get_edges() const {
            return g.get_edges();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_node */
        bool has_node(const N& n) const {
            return g.has_node(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_edge */
        bool has_edge(const E& e) const {
            return g.has_edge(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs_full */
        full_input_output_range<N, E, ED> auto get_outputs_full(const N& n) const {
            return g.get_inputs_full(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs_full */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const {
            return g.get_outputs_full(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const {
            return g.get_inputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const {
            return g.get_outputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_outputs */
        bool has_outputs(const N& n) const {
            return g.has_inputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_inputs */
        bool has_inputs(const N& n) const {
            return g.has_outputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_out_degree */
        std::size_t get_out_degree(const N& n) const {
            return g.get_in_degree(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_in_degree */
        std::size_t get_in_degree(const N& n) const {
            return g.get_out_degree(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::edge_to_element_offsets */
        std::optional<std::pair<std::optional<N_INDEX>, std::optional<N_INDEX>>> edge_to_element_offsets(
            const E& e
        ) const {
            auto offset { g.edge_to_element_offsets(e) };
            if (!offset.has_value()) {
                return std::nullopt;
            }
            auto [v_idx, w_idx] { *offset };
            if (v_idx.has_value()) {
                *v_idx = static_cast<N_INDEX>(g.grid_down_cnt - *v_idx - I1);  // Cast to prevent narrowing warning
            }
            if (w_idx.has_value()) {
                *w_idx = static_cast<N_INDEX>(g.grid_right_cnt - *w_idx - I1);  // Cast to prevent narrowing warning
            }
            return offset;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_to_grid_offset */
        std::tuple<N_INDEX, N_INDEX, std::size_t> node_to_grid_offset(const N& n) const {
            const auto& [grid_down, grid_right, depth] { g.node_to_grid_offset(n) };
            return {
                static_cast<N_INDEX>(grid_down_cnt - grid_down - I1),  // Cast to prevent narrowing warning
                static_cast<N_INDEX>(grid_right_cnt - grid_right - I1),  // Cast to prevent narrowing warning
                depth
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_offset_to_nodes */
        bidirectional_range_of_non_cvref<N> auto grid_offset_to_nodes(N_INDEX grid_down, N_INDEX grid_right) const {
            if constexpr (debug_mode) {
                if (grid_down >= grid_down_cnt || grid_right >= grid_right_cnt) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            return g.grid_offset_to_nodes(
                static_cast<N_INDEX>(grid_down_cnt - grid_down - I1),  // Cast to prevent narrowing warning
                static_cast<N_INDEX>(grid_right_cnt - grid_right - I1)  // Cast to prevent narrowing warning
            );
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down) const {
            return
                g.row_nodes(
                    static_cast<N_INDEX>(grid_down_cnt - grid_down - I1)  // Cast to prevent narrowing warning
                )
                | std::views::reverse;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down, const N& root_node, const N& leaf_node) const {
            return
                g.row_nodes(
                    static_cast<N_INDEX>(grid_down_cnt - grid_down - I1),  // Cast to prevent narrowing warning
                    leaf_node,
                    root_node
                )
                | std::views::reverse;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::is_reachable */
        bool is_reachable(const N& n1, const N& n2) const {
            return g.is_reachable(n2, n1);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes */
        bidirectional_range_of_non_cvref<N> auto resident_nodes() const {
            return g.resident_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::outputs_to_residents */
        bidirectional_range_of_non_cvref<E> auto outputs_to_residents(const N& n) const {
            return g.inputs_from_residents(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::inputs_from_residents */
        bidirectional_range_of_non_cvref<E> auto inputs_from_residents(const N& n) const {
            return g.outputs_to_residents(n);
        }
    };


    /**
     * Create @ref offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph
     * instance, where template parameters are deduced / inferred from arguments passed in.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @return New @ref offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph
     *      instance.
     */
    template<
        bool debug_mode
    >
    auto create_reversed_sliceable_pairwise_alignment_graph(
        const sliceable_pairwise_alignment_graph auto& g
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        return reversed_sliceable_pairwise_alignment_graph<debug_mode, G> { g };
    }
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
