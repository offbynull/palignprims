#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <functional>
#include <type_traits>
#include <stdfloat>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_parwise_alignment_graph GRAPH
    >
    class reversed_sliceable_pairwise_alignment_graph {
    public:
        using INDEX = typename GRAPH::INDEX;
        using N = typename GRAPH::N;
        using E = typename GRAPH::E;
        using ED = typename GRAPH::ED;
        using ND = typename GRAPH::ND;

    private:
        GRAPH& g;

    public:
        const INDEX down_node_cnt;
        const INDEX right_node_cnt;

        reversed_sliceable_pairwise_alignment_graph(GRAPH& _g)
        : g{_g}
        , down_node_cnt{_g.down_node_cnt}
        , right_node_cnt{_g.right_node_cnt} {}

        void update_node_data(const N& node, ND&& data) {
            g.update_node_data(node, std::forward<ND>(data));
        }

        ND& get_node_data(const N& node) {
            return g.get_node_data(node);
        }

        void update_edge_data(const E& edge, ED&& data) {
            g.update_edge_data(edge, std::forward<ED>(data));
        }

        ED& get_edge_data(const E& edge) {
            return g.get_edge_data(edge);
        }

        N get_edge_from(const E& edge) {
            return g.get_edge_to(edge);
        }

        N get_edge_to(const E& edge) {
            return g.get_edge_from(edge);
        }

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            return g.get_edge(edge);
        }

        auto get_root_nodes() {
            return g.get_leaf_nodes();
        }

        N get_root_node() {
            return g.get_leaf_node();
        }

        auto get_leaf_nodes() {
            return g.get_root_nodes();
        }

        auto get_leaf_node() {
            return g.get_root_node();
        }

        auto get_nodes() {
            return g.get_nodes();
        }

        auto get_edges() {
            return g.get_edges();
        }

        bool has_node(const N& node) {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) {
            return g.has_edge(edge);
        }

        auto get_outputs_full(const N& node) {
            return g.get_inputs_full(node);
        }

        auto get_inputs_full(const N& node) {
            return g.get_outputs_full(node);
        }

        auto get_outputs(const N& node) {
            return g.get_inputs(node);
        }

        auto get_inputs(const N& node) {
            return g.get_outputs(node);
        }

        bool has_outputs(const N& node) {
            return g.has_inputs(node);
        }

        bool has_inputs(const N& node) {
            return g.has_outputs(node);
        }

        std::size_t get_out_degree(const N& node) {
            return g.get_in_degree(node);
        }

        std::size_t get_in_degree(const N& node) {
            return g.get_out_degree(node);
        }

        template<weight WEIGHT=std::float64_t>
        void assign_weights(
            const std::ranges::random_access_range auto& v,  // random access container
            const std::ranges::random_access_range auto& w,  // random access container
            std::function<
                WEIGHT(
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
                )
            > weight_lookup,
            std::function<void(ED&, WEIGHT weight)> weight_setter
        ) {
            // TODO: Make a concept that doesn't contain this func and use that instead for the code that requires this class (sliced walking)
            throw std::runtime_error("Unsupported");
        }

        std::optional<std::pair<std::optional<INDEX>, std::optional<INDEX>>> edge_to_element_offsets(
            const E& edge
        ) {
            auto offset { g.edge_to_element_offsets(edge) };
            if (!offset.has_element()) {
                return std::nullopt;
            }
            auto [v_idx, w_idx] { offset };
            if (v_idx.has_element()) {
                *v_idx = g.down_node_cnt - *v_idx - 1u;
            }
            if (w_idx.has_element()) {
                *w_idx = g.right_node_cnt - *w_idx - 1u;
            }
            return offset;
        }

        std::pair<INDEX, INDEX> node_to_grid_offsets(const N& node) {
            const auto& [n_down, n_right] { g.node_to_grid_offsets(node) };
            return { down_node_cnt - n_down - 1u, right_node_cnt - n_right - 1u };
        }

        constexpr static INDEX node_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return GRAPH::node_count(_down_node_cnt, _right_node_cnt);
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return GRAPH::longest_path_edge_count(_down_node_cnt, _right_node_cnt);
        }

        static std::size_t slice_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return GRAPH::slice_nodes_capacity(_down_node_cnt, _right_node_cnt);
        }

        auto slice_nodes(INDEX n_down) {
            return g.slice_nodes(n_down);
        }

        N first_node_in_slice(INDEX n_down) {
            return g.last_node_in_slice(n_down);
        }

        N last_node_in_slice(INDEX n_down) {
            return g.first_node_in_slice(n_down);
        }

        N next_node_in_slice(const N& node) {
            return g.prev_node_in_slice(node);
        }

        N prev_node_in_slice(const N& node) {
            return g.next_node_in_slice(node);
        }

        static std::size_t resident_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return GRAPH::resident_nodes_capacity(_down_node_cnt, _right_node_cnt);
        }

        auto resident_nodes() {
            return g.resident_nodes();
        }

        auto outputs_to_residents(const N& node) {
            return g.inputs_from_residents(node);
        }

        auto inputs_from_residents(const N& node) {
            return g.outputs_to_residents(node);
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
