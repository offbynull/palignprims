#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <tuple>
#include <stdexcept>
#include <optional>
#include <utility>
#include <type_traits>
#include <functional>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::pairwise_global_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::node;
    using offbynull::aligner::graphs::grid_graph::edge;
    using offbynull::aligner::graphs::grid_graph::empty_node_data;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::aligner::graph::graph::full_input_output_range;

    /**
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph implementation of a
     * pairwise global sequence alignment graph.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX_ Node coordinate type.
     * @tparam WEIGHT Edge data type (edge's weight).
     * @tparam DOWN_SEQ Downward sequence type.
     * @tparam RIGHT_SEQ Rightward sequence type.
     * @tparam SUBSTITUTION_SCORER Scorer type used to score sequence alignment substitutions.
     * @tparam GAP_SCORER Scorer type to score for sequence alignment gaps (indels).
     */
    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX_,
        weight WEIGHT,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > SUBSTITUTION_SCORER,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > GAP_SCORER
    >
    class pairwise_global_alignment_graph {
    private:
        static constexpr N_INDEX_ I0 { static_cast<N_INDEX_>(0zu) };
        static constexpr N_INDEX_ I1 { static_cast<N_INDEX_>(1zu) };
        static constexpr N_INDEX_ I2 { static_cast<N_INDEX_>(2zu) };

        const grid_graph<
            debug_mode,
            N_INDEX_,
            WEIGHT,
            DOWN_SEQ,
            RIGHT_SEQ,
            SUBSTITUTION_SCORER,
            GAP_SCORER
        > g;

    public:
        /** Element object type of downward sequence (CV-qualification and references removed). */
        using DOWN_ELEM = std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>;
        /** Element object type of rightward sequence (CV-qualification and references removed). */
        using RIGHT_ELEM = std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N_INDEX */
        using N_INDEX = N_INDEX_;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N */
        using N = node<N_INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::E */
        using E = edge<N_INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ND */
        using ND = empty_node_data;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ED */
        using ED = WEIGHT;

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_down_cnt */
        const N_INDEX grid_down_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_right_cnt */
        const N_INDEX grid_right_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_depth_cnt */
        static constexpr N_INDEX grid_depth_cnt { decltype(g)::grid_depth_cnt };  // 0
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes_capacity */
        static constexpr std::size_t resident_nodes_capacity { decltype(g)::resident_nodes_capacity };  // 0
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::path_edge_capacity */
        const std::size_t path_edge_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_incoming_edge_capacity */
        const std::size_t node_incoming_edge_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_outgoing_edge_capacity */
        const std::size_t node_outgoing_edge_capacity;

        // Scorer params are not being made into universal references because there's a high chance of enabling a subtle bug: There's a
        // non-trivial possibility that the user will submit the same object for both scorers, and so if the universal reference ends up
        // being an rvalue reference it'll try to move the same object twice.
        /**
         * Construct an @ref offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph instance.
         *
         * @param down_seq_ Downward sequence.
         * @param right_seq_ Rightward sequence.
         * @param substitution_scorer_ Scorer for sequence alignment substitutions.
         * @param gap_scorer_ Scorer for sequence alignment gaps (indels).
         */
        pairwise_global_alignment_graph(
            const DOWN_SEQ& down_seq_,
            const RIGHT_SEQ& right_seq_,
            const SUBSTITUTION_SCORER& substitution_scorer_,
            const GAP_SCORER& gap_scorer_
        )
        : g { down_seq_, right_seq_, substitution_scorer_, gap_scorer_ }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt }
        , path_edge_capacity { g.path_edge_capacity }
        , node_incoming_edge_capacity { g.node_incoming_edge_capacity }
        , node_outgoing_edge_capacity { g.node_outgoing_edge_capacity } {}

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
            return g.get_edge_from(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_to */
        N get_edge_to(const E& e) const {
            return g.get_edge_to(e);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge */
        std::tuple<N, N, ED> get_edge(const E& e) const {
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
            return g.get_leaf_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_node */
        N get_leaf_node() const {
            return g.get_leaf_node();
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
            return g.get_outputs_full(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs_full */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const {
            return g.get_inputs_full(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const {
            return g.get_outputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const {
            return g.get_inputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_outputs */
        bool has_outputs(const N& n) const {
            return g.has_outputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_inputs */
        bool has_inputs(const N& n) const {
            return g.has_inputs(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_out_degree */
        std::size_t get_out_degree(const N& n) const {
            return g.get_out_degree(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_in_degree */
        std::size_t get_in_degree(const N& n) const {
            return g.get_in_degree(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::edge_to_element_offsets */
        std::optional<
            std::pair<
                std::optional<N_INDEX>,
                std::optional<N_INDEX>
            >
        > edge_to_element_offsets(
            const E& e
        ) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            using OPT_INDEX = std::optional<N_INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            const auto& [n1, n2] { e };
            const auto& [n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_grid_down, n2_grid_right] { n2 };
            if (n1_grid_down + I1 == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                return RET { { { n1_grid_down }, { n1_grid_right } } };
            } else if (n1_grid_down + I1 == n2_grid_down && n1_grid_right == n2_grid_right) {
                return RET { { { n1_grid_down }, std::nullopt } };
            } else if (n1_grid_down == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                return RET { { std::nullopt, { n1_grid_right } } };
            }
            if constexpr (debug_mode) {
                throw std::runtime_error { "This should never happen" };
            }
            std::unreachable();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_to_grid_offset */
        std::tuple<N_INDEX, N_INDEX, std::size_t> node_to_grid_offset(const N& n) const {
            return g.node_to_grid_offset(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_offset_to_nodes */
        bidirectional_range_of_non_cvref<N> auto grid_offset_to_nodes(N_INDEX grid_down, N_INDEX grid_right) const {
            return g.grid_offset_to_nodes(grid_down, grid_right);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down) const {
            return g.row_nodes(grid_down);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down, const N& root_node, const N& leaf_node) const {
            return g.row_nodes(grid_down, root_node, leaf_node);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::is_reachable */
        bool is_reachable(const N& n1, const N& n2) const {
            return g.is_reachable(n1, n2);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes */
        bidirectional_range_of_non_cvref<N> auto resident_nodes() const {
            return g.resident_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::outputs_to_residents */
        bidirectional_range_of_non_cvref<E> auto outputs_to_residents(const N& n) const {
            return g.outputs_to_residents(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::inputs_from_residents */
        bidirectional_range_of_non_cvref<E> auto inputs_from_residents(const N& n) const {
            return g.inputs_from_residents(n);
        }
    };

    /**
     * Create @ref offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph instance, where template
     * parameters are deduced / inferred from arguments passed in.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX Node coordinate type.
     * @param down_seq Downward sequence.
     * @param right_seq Rightward sequence.
     * @param substitution_scorer Scorer for sequence alignment substitutions.
     * @param gap_scorer Scorer for sequence alignment gaps (indels).
     * @return New @ref offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph instance.
     */
    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX
    >
    auto create_pairwise_global_alignment_graph(
        const sequence auto& down_seq,
        const sequence auto& right_seq,
        const scorer_without_explicit_weight<
            N_INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& substitution_scorer,
        const scorer_without_explicit_weight<
            N_INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& gap_scorer
    ) {
        using DOWN_SEQ = std::remove_cvref_t<decltype(down_seq)>;
        using DOWN_ELEM = std::remove_cvref_t<decltype(down_seq[0zu])>;
        using RIGHT_SEQ = std::remove_cvref_t<decltype(right_seq)>;
        using RIGHT_ELEM = std::remove_cvref_t<decltype(right_seq[0zu])>;
        using WEIGHT_1 = decltype(
            substitution_scorer(
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        using WEIGHT_2 = decltype(
            gap_scorer(
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        static_assert(std::is_same_v<WEIGHT_1, WEIGHT_2>, "Scorers must return the same weight type");
        return pairwise_global_alignment_graph<
            debug_mode,
            N_INDEX,
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
