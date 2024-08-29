#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <tuple>
#include <stdexcept>
#include <optional>
#include <utility>
#include <type_traits>
#include <functional>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::pairwise_global_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::node;
    using offbynull::aligner::graphs::grid_graph::edge;
    using offbynull::aligner::graphs::grid_graph::empty_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;

    template<
        bool debug_mode,
        widenable_to_size_t INDEX_,
        weight WEIGHT,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        scorer<
            edge<INDEX_>,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > SUBSTITUTION_SCORER,
        scorer<
            edge<INDEX_>,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > GAP_SCORER
    >
    class pairwise_global_alignment_graph {
    private:
        const grid_graph<
            debug_mode,
            INDEX_,
            WEIGHT,
            DOWN_SEQ,
            RIGHT_SEQ,
            SUBSTITUTION_SCORER,
            GAP_SCORER
        > g;

    public:
        using DOWN_ELEM = std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = node<INDEX>;
        using E = edge<INDEX>;
        using ND = empty_type;
        using ED = WEIGHT;

        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;
        static constexpr INDEX grid_depth_cnt { decltype(g)::grid_depth_cnt };  // 0
        static constexpr std::size_t resident_nodes_capacity { decltype(g)::resident_nodes_capacity };  // 0
        const std::size_t path_edge_capacity;

        // Scorer params are not being made into universal references because there's a high chance of enabling a subtle bug: There's a
        // non-trivial possibility that the user will submit the same object for both scorers, and so if the universal reference ends up
        // being an rvalue reference it'll try to move the same object twice.
        pairwise_global_alignment_graph(
            const DOWN_SEQ& down_seq_,
            const RIGHT_SEQ& right_seq_,
            const SUBSTITUTION_SCORER& substitution_scorer_,
            const GAP_SCORER& gap_scorer_
        )
        : g { down_seq_, right_seq_, substitution_scorer_, gap_scorer_ }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt }
        , path_edge_capacity { g.path_edge_capacity } {}

        ND get_node_data(const N& node) const {
            return g.get_node_data(node);
        }

        ED get_edge_data(const E& edge) const {
            return g.get_edge_data(edge);
        }

        N get_edge_from(const E& edge) const {
            return g.get_edge_from(edge);
        }

        N get_edge_to(const E& edge) const {
            return g.get_edge_to(edge);
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            return g.get_edge(edge);
        }

        auto get_root_nodes() const {
            return g.get_root_nodes();
        }

        N get_root_node() const {
            return g.get_root_node();
        }

        auto get_leaf_nodes() const {
            return g.get_leaf_nodes();
        }

        N get_leaf_node() const {
            return g.get_leaf_node();
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
            return g.get_outputs_full(node);
        }

        auto get_inputs_full(const N& node) const {
            return g.get_inputs_full(node);
        }

        auto get_outputs(const N& node) const {
            return g.get_outputs(node);
        }

        auto get_inputs(const N& node) const {
            return g.get_inputs(node);
        }

        bool has_outputs(const N& node) const {
            return g.has_outputs(node);
        }

        bool has_inputs(const N& node) const {
            return g.has_inputs(node);
        }

        std::size_t get_out_degree(const N& node) const {
            return g.get_out_degree(node);
        }

        std::size_t get_in_degree(const N& node) const {
            return g.get_in_degree(node);
        }

        auto edge_to_element_offsets(
            const E& edge
        ) const {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            const auto& [n1, n2] { edge };
            const auto& [n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_grid_down, n2_grid_right] { n2 };
            if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { { n1_grid_down }, { n1_grid_right } } };
            } else if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                return RET { { { n1_grid_down }, std::nullopt } };
            } else if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { std::nullopt, { n1_grid_right } } };
            }
            if constexpr (debug_mode) {
                throw std::runtime_error { "Bad edge" };
            }
            std::unreachable();
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            return g.node_to_grid_offsets(node);
        }

        auto slice_nodes(INDEX grid_down) const {
            return g.slice_nodes(grid_down);
        }

        auto slice_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            return g.slice_nodes(grid_down, root_node, leaf_node);
        }

        bool is_reachable(const N& n1, const N& n2) const {
            return g.is_reachable(n1, n2);
        }

        auto resident_nodes() const {
            return g.resident_nodes();
        }

        auto outputs_to_residents(const N& node) const {
            return g.outputs_to_residents(node);
        }

        auto inputs_from_residents(const N& node) const {
            return g.inputs_from_residents(node);
        }
    };

    template<
        bool debug_mode,
        widenable_to_size_t INDEX
    >
    auto create_pairwise_global_alignment_graph(
        const sequence auto& down_seq,
        const sequence auto& right_seq,
        const scorer_without_explicit_weight<
            edge<INDEX>,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& substitution_scorer,
        const scorer_without_explicit_weight<
            edge<INDEX>,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& gap_scorer
    ) {
        using DOWN_SEQ = std::remove_cvref_t<decltype(down_seq)>;
        using DOWN_ELEM = std::remove_cvref_t<decltype(down_seq[0zu])>;
        using RIGHT_SEQ = std::remove_cvref_t<decltype(down_seq)>;
        using RIGHT_ELEM = std::remove_cvref_t<decltype(right_seq[0zu])>;
        using WEIGHT_1 = decltype(
            substitution_scorer(
                std::declval<const edge<INDEX>&>(),
                std::declval<const std::optional<std::reference_wrapper<const DOWN_ELEM>>>(),
                std::declval<const std::optional<std::reference_wrapper<const RIGHT_ELEM>>>()
            )
        );
        using WEIGHT_2 = decltype(
            gap_scorer(
                std::declval<const edge<INDEX>&>(),
                std::declval<const std::optional<std::reference_wrapper<const DOWN_ELEM>>>(),
                std::declval<const std::optional<std::reference_wrapper<const RIGHT_ELEM>>>()
            )
        );
        static_assert(std::is_same_v<WEIGHT_1, WEIGHT_2>, "Scorers must return the same weight type");
        return pairwise_global_alignment_graph<
            debug_mode,
            INDEX,
            WEIGHT_1,
            DOWN_SEQ,
            RIGHT_SEQ,
            std::remove_cvref_t<decltype(substitution_scorer)>,
            std::remove_cvref_t<decltype(gap_scorer)>
        > {
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer
        };
    }
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
