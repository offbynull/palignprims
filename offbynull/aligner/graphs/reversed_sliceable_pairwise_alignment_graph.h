#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_REVERSED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <functional>
#include <type_traits>
#include <stdfloat>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::pairwise_global_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_parwise_alignment_graph GRAPH
    >
    class pairwise_global_alignment_graph {
    public:
        using INDEX = typename GRAPH::INDEX_;
        using N = typename GRAPH::N;
        using E = typename GRAPH::E;
        using ED = typename GRAPH::ED;
        using ND = typename GRAPH::ND;

    private:
        GRAPH& g;

    public:
        const INDEX down_node_cnt;
        const INDEX right_node_cnt;

        pairwise_global_alignment_graph(GRAPH& _g)
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

        std::tuple<E, N, N, ED&> get_output_full(const N& node) {
            return g.get_input_full(node);
        }

        auto get_inputs_full(const N& node) {
            return g.get_outputs_full(node);
        }

        std::tuple<E, N, N, ED&> get_input_full(const N& node) {
            return g.get_output_full(node);
        }

        auto get_outputs(const N& node) {
            return g.get_inputs(node);
        }

        E get_output(const N& node) {
            return g.get_input(node);
        }

        auto get_inputs(const N& node) {
            return g.get_outputs(node);
        }

        E get_input(const N& node) {
            return g.get_output(node);
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
            g.assign_weights(v, w, weight_lookup, weight_setter);
        }

        static auto edge_to_elements(
            const E& edge,
            const auto& v,  // random access container
            const auto& w   // random access container
        ) {
            return GRAPH::edge_to_elements(edge, v, w);
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

        std::size_t max_slice_nodes_count() {
            return g.max_slice_nodes_count();
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

        std::size_t max_resident_nodes_count() {
            return g.max_resident_nodes_count();
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
