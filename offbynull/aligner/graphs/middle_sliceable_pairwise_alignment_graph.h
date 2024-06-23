#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_MIDDLE_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_MIDDLE_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <ranges>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_parwise_alignment_graph GRAPH,
        bool error_check=true
    >
    class middle_sliceable_pairwise_alignment_graph {
    public:
        using INDEX = typename GRAPH::INDEX;
        using N = typename GRAPH::N;
        using E = typename GRAPH::E;
        using ED = typename GRAPH::ED;
        using ND = typename GRAPH::ND;

    private:
        prefix_sliceable_pairwise_alignment_graph<
            GRAPH,
            error_check
        > inner_g;
        suffix_sliceable_pairwise_alignment_graph<
            prefix_sliceable_pairwise_alignment_graph<
                GRAPH,
                error_check
            >,
            error_check
        > g;

    public:
        const INDEX grid_down_offset;
        const INDEX grid_right_offset;
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        middle_sliceable_pairwise_alignment_graph(
            GRAPH& _g,
            INDEX grid_down_offset,
            INDEX grid_right_offset,
            INDEX grid_down_cnt,
            INDEX grid_right_cnt
        )
        : inner_g {
            _g,
            grid_down_offset + grid_down_cnt,
            grid_right_offset + grid_right_cnt
        }
        , g {
            inner_g,
            grid_down_cnt,
            grid_right_cnt
        }
        , grid_down_offset { grid_down_offset }
        , grid_right_offset { grid_right_offset }
        , grid_down_cnt { grid_down_cnt }
        , grid_right_cnt { grid_right_cnt } { }

        static middle_sliceable_pairwise_alignment_graph<GRAPH, error_check> create_using_offsets(
            GRAPH& g,
            INDEX down_offset_1,
            INDEX right_offset_1,
            INDEX down_offset_2,
            INDEX right_offset_2
        ) {
            if constexpr (error_check) {
                if (down_offset_1 > down_offset_2 || right_offset_1 > right_offset_2) {
                    throw std::runtime_error("Top-left isn't top-left");
                }
            }
            return middle_sliceable_pairwise_alignment_graph<GRAPH, error_check> {
                g,
                down_offset_1,
                right_offset_1,
                down_offset_2 - down_offset_1 + 1u,
                right_offset_2 - right_offset_1 + 1u
            };
        }

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

        std::pair<INDEX, INDEX> node_to_grid_offsets(const N& node) {
            return g.node_to_grid_offsets(node);
        }

        constexpr static INDEX node_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return GRAPH::node_count(_grid_down_cnt, _grid_right_cnt);
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return GRAPH::longest_path_edge_count(_grid_down_cnt, _grid_right_cnt);
        }

        constexpr static std::size_t slice_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
            return GRAPH::slice_nodes_capacity(_grid_down_cnt, _grid_right_cnt);
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

        constexpr static std::size_t resident_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
            return GRAPH::resident_nodes_capacity(_grid_down_cnt, _grid_right_cnt);
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
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_MIDDLE_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
