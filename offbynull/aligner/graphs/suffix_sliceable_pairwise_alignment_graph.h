#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_SUFFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_SUFFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <ranges>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        bool error_check=true
    >
    class suffix_sliceable_pairwise_alignment_graph {
    public:
        using INDEX = typename G::INDEX;
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using ND = typename G::ND;

    private:
        reversed_sliceable_pairwise_alignment_graph<
            G,
            error_check
        > inner_inner_g;
        prefix_sliceable_pairwise_alignment_graph<
            reversed_sliceable_pairwise_alignment_graph<
                G,
                error_check
            >,
            error_check
        > inner_g;
        reversed_sliceable_pairwise_alignment_graph<
            prefix_sliceable_pairwise_alignment_graph<
                reversed_sliceable_pairwise_alignment_graph<
                    G,
                    error_check
                >,
                error_check
            >,
            error_check
        > g;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        suffix_sliceable_pairwise_alignment_graph(
            G& g_,
            const N& new_root_node_
        )
        : inner_inner_g{ g_ }
        , inner_g { inner_inner_g, new_root_node_ }
        , g { inner_g }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt } {}

        // The first implementation of this was proxing the exact type. For example...
        //
        // using get_edge_to_ret_type = decltype(g.get_edge_to(std::declval<E>()));
        // get_edge_to_ret_type get_edge_to(const E &edge) {
        //     return g.get_edge_to(...);
        // }
        //
        // Decided not to do this. No pairwise graph type uses ND&/ED&/N&/E&, so just go ahead and return concrete
        // object.

        ND get_node_data(const N& node) {
            return g.get_node_data(node);
        }

        ED get_edge_data(const E& edge) {
            return g.get_edge_data(edge);
        }

        N get_edge_from(const E& edge) {
            return g.get_edge_from(edge);
        }

        N get_edge_to(const E& edge) {
            return g.get_edge_to(edge);
        }

        std::tuple<N, N, ED> get_edge(const E& edge) {
            return g.get_edge(edge);
        }

        auto get_root_nodes() {
            return g.get_root_nodes();
        }

        N get_root_node() {
            return g.get_root_node();
        }

        auto get_leaf_nodes() {
            return g.get_leaf_nodes();
        }

        N get_leaf_node() {
            return g.get_leaf_node();
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
            return g.get_outputs_full(node);
        }

        auto get_inputs_full(const N& node) {
            return g.get_inputs_full(node);
        }

        auto get_outputs(const N& node) {
            return g.get_outputs(node);
        }

        auto get_inputs(const N& node) {
            return g.get_inputs(node);
        }

        bool has_outputs(const N& node) {
            return g.has_outputs(node);
        }

        bool has_inputs(const N& node) {
            return g.has_inputs(node);
        }

        std::size_t get_out_degree(const N& node) {
            return g.get_out_degree(node);
        }

        std::size_t get_in_degree(const N& node) {
            return g.get_in_degree(node);
        }

        std::optional<std::pair<std::optional<INDEX>, std::optional<INDEX>>> edge_to_element_offsets(const E& edge) {
            return g.edge_to_element_offsets(edge);
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) {
            return g.node_to_grid_offsets(node);
        }

        constexpr static auto limits(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return G::limits(_grid_down_cnt, _grid_right_cnt);;
        }

        auto slice_nodes(INDEX grid_down) {
            return g.slice_nodes(grid_down);
        }

        auto slice_nodes(INDEX grid_down, INDEX override_grid_right_cnt) {
            return g.slice_nodes(grid_down, override_grid_right_cnt);
        }

        N slice_first_node(INDEX grid_down) {
            return g.slice_first_node(grid_down);
        }

        N slice_first_node(INDEX grid_down, INDEX grid_right) {
            return g.slice_first_node(grid_down, grid_right);
        }

        N slice_last_node(INDEX grid_down) {
            return g.slice_last_node(grid_down);
        }

        N slice_last_node(INDEX grid_down, INDEX grid_right) {
            return g.slice_last_node(grid_down, grid_right);
        }

        N slice_next_node(const N& node) {
            return g.slice_next_node(node);
        }

        N slice_prev_node(const N& node) {
            return g.slice_prev_node(node);
        }

        auto resident_nodes() {
            return g.resident_nodes();
        }

        auto outputs_to_residents(const N& node) {
            return g.outputs_to_residents(node);
        }

        auto inputs_from_residents(const N& node) {
            return g.inputs_from_residents(node);
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_SUFFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
