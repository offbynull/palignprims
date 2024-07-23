#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <functional>
#include <type_traits>
#include <stdfloat>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::pairwise_global_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::empty_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;

    template<
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t,
        bool error_check = true
    >
    class pairwise_global_alignment_graph {
    public:
        using DOWN_ELEM = std::decay_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = std::pair<INDEX, INDEX>;
        using E = std::pair<N, N>;
        using ND = empty_type;
        using ED = WEIGHT;  // Differs from backing grid_graph because these values are derived at time of access

    private:
        grid_graph<
            DOWN_SEQ,
            RIGHT_SEQ,
            INDEX,
            WEIGHT,
            error_check
        > g;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        pairwise_global_alignment_graph(
            const DOWN_SEQ& _down_seq,
            const RIGHT_SEQ& _right_seq,
            std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _match_lookup,
            std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _indel_lookup
        )
        : g{_down_seq, _right_seq, _match_lookup, _indel_lookup}
        , grid_down_cnt{g.grid_down_cnt}
        , grid_right_cnt{g.grid_right_cnt} {}

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

        auto edge_to_element_offsets(
            const E& edge
        ) {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            const auto& [n1, n2] {edge};
            const auto& [n1_grid_down, n1_grid_right] {n1};
            const auto& [n2_grid_down, n2_grid_right] {n2};
            if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { { n1_grid_down }, { n1_grid_right } } };
            } else if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                return RET { { { n1_grid_down }, std::nullopt } };
            } else if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { std::nullopt, { n1_grid_right } } };
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        constexpr static auto limits(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return decltype(g)::limits(_grid_down_cnt, _grid_right_cnt);;
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) {
            return g.node_to_grid_offsets(node);
        }

        auto slice_nodes(INDEX grid_down) {
            return g.slice_nodes(grid_down);
        }

        auto slice_nodes(INDEX grid_down, INDEX grid_right_cnt_) {
            return g.slice_nodes(grid_down, grid_right_cnt_);
        }

        N slice_first_node(INDEX grid_down) {
            return g.slice_first_node(grid_down);
        }

        N slice_first_node(INDEX grid_down, INDEX grid_right_cnt_) {
            return g.slice_first_node(grid_down, grid_right_cnt_);
        }

        N slice_last_node(INDEX grid_down) {
            return g.slice_last_node(grid_down);
        }

        N slice_last_node(INDEX grid_down, INDEX grid_right_cnt_) {
            return g.slice_last_node(grid_down, grid_right_cnt_);
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
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
