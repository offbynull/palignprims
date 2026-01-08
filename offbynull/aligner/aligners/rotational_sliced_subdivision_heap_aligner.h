#ifndef OFFBYNULL_ALIGNER_ALIGNERS_ROTATIONAL_SLICED_SUBDIVISION_HEAP_ALIGNER_H
#define OFFBYNULL_ALIGNER_ALIGNERS_ROTATIONAL_SLICED_SUBDIVISION_HEAP_ALIGNER_H

#include <utility>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <optional>
#include <tuple>
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/repeat_sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/widening_scorer.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker/forward_walker.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/aligners/concepts.h"
#include "offbynull/aligner/aligners/internal_utils.h"

namespace offbynull::aligner::aligners::rotational_sliced_subdivision_heap_aligner {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::numeric;
    using offbynull::utils::wider_numeric;
    using offbynull::utils::compile_time_constant;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight_or_seq_index;
    using offbynull::aligner::scorers::widening_scorer::widening_scorer;
    using offbynull::aligner::sequences::repeat_sequence::repeat_sequence;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge_type;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_heap_container_creator_pack
        ::backtracker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker
        ::forward_walker_heap_container_creator_pack::forward_walker_heap_container_creator_pack;
    using offbynull::aligner::aligners::concepts::alignment_result;
    using offbynull::aligner::aligners::internal_utils::validate_align_and_extract_widest_weight;

    /**
     * Rotational pairwise aligner using sliced subdivision algorithm (see
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker) and heap allocation.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     */
    template<bool debug_mode>
    class rotational_sliced_subdivision_heap_aligner {
    public:

        /**
         * Align two sequences.
         *
         * @tparam minimize_allocations `true` to force certain internal containers to reserve (preallocate) elements thereby
         *     removing/reducing the need for adhoc reallocations, `false` otherwise.
         * @tparam N_INDEX Alignment graph node coordinate type (must be wide enough to hold `std::max(down.size(), right.size()) + 1zu`).
         * @param down Sequence 1 (sequence to rotate).
         * @param right Sequence 2 (sequence to fit).
         * @param substitution_scorer Match/mismatch (substitution) scorer.
         * @param gap_scorer Gap (indel) scorer.
         * @param freeride_scorer Freeride scorer.
         * @param max_path_weight_comparison_tolerance Tolerance used when testing for weight for equality. This may need to be non-zero
         *     when the type used for edge weights is a floating point type (must be finite). It helps mitigate floating point rounding
         *     errors when sequences are large / when scoring has large magnitude differences. The value this should be set to depends
         *     on multiple factors (e.g., which floating point type is used, expected graph size, expected magnitudes, etc..).
         * @return Rotational alignment of `down` vs `right` along with the score of that alignment.
         */
        template<
            bool minimize_allocations = true,
            widenable_to_size_t N_INDEX = std::size_t
        >
        alignment_result auto align(
            const sequence auto& down,
            const sequence auto& right,
            const scorer_without_explicit_weight_or_seq_index<
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>
            > auto& substitution_scorer,
            const scorer_without_explicit_weight_or_seq_index<
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>
            > auto& gap_scorer,
            const scorer_without_explicit_weight_or_seq_index<
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>
            > auto& freeride_scorer,
            const weight auto max_path_weight_comparison_tolerance
        ) {
            auto widest_weight_type_tagger {
                validate_align_and_extract_widest_weight<
                    debug_mode,
                    N_INDEX,
                    std::remove_cvref_t<decltype(substitution_scorer)>,
                    std::remove_cvref_t<decltype(gap_scorer)>,
                    std::remove_cvref_t<decltype(freeride_scorer)>
                >(down, right)
            };
            using WEIGHT = typename decltype(widest_weight_type_tagger)::type;

            // Check max_path_weight_comparison_tolerance doesn't narrow
            using WEIGHT_TOLERANCE = decltype(max_path_weight_comparison_tolerance);
            static_assert(numeric<WEIGHT_TOLERANCE>, "weight must be numeric");
            if constexpr (debug_mode) {
                if (max_path_weight_comparison_tolerance != static_cast<WEIGHT>(max_path_weight_comparison_tolerance)) {
                    throw std::runtime_error { "Max path weight tolerance not representable by WEIGHT type" };
                }
            }

            // Wrap scorers into ones that'll cast from N_INDEX to the scorer's SEQ_INDEX (widens, doesn't narrow). Likewise, the
            // wrapped scorer's result will be cast to WEIGHT type derived above (widens). This is to make it easier for the user
            // (e.g., they submit scorers with mismatching N_INDEX / WEIGHT).
            using DOWN_ELEM = std::remove_cvref_t<decltype(down[0])>;
            using RIGHT_ELEM = std::remove_cvref_t<decltype(right[0])>;
            const widening_scorer<
                debug_mode,
                N_INDEX,
                DOWN_ELEM,
                RIGHT_ELEM,
                WEIGHT,
                std::remove_cvref_t<decltype(substitution_scorer)>
            > widened_substitution_scorer {
                substitution_scorer
            };
            const widening_scorer<
                debug_mode,
                N_INDEX,
                DOWN_ELEM,
                RIGHT_ELEM,
                WEIGHT,
                std::remove_cvref_t<decltype(gap_scorer)>
            > widened_gap_scorer {
                gap_scorer
            };
            const widening_scorer<
                debug_mode,
                N_INDEX,
                DOWN_ELEM,
                RIGHT_ELEM,
                WEIGHT,
                std::remove_cvref_t<decltype(freeride_scorer)>
            > widened_freeride_scorer {
                freeride_scorer
            };

            return align_strict<N_INDEX, WEIGHT, minimize_allocations>(
                down,
                right,
                widened_substitution_scorer,
                widened_gap_scorer,
                widened_freeride_scorer,
                max_path_weight_comparison_tolerance
            );
        }

        /**
         * Align two sequences.
         *
         * @tparam minimize_allocations `true` to force certain internal containers to reserve (preallocate) elements thereby
         *     removing/reducing the need for adhoc reallocations, `false` otherwise.
         * @tparam WEIGHT Alignment graph edge weight type.
         * @tparam N_INDEX Alignment graph node coordinate type (must be wide enough to hold `std::max(down.size(), right.size()) + 1zu`).
         * @param down Sequence 1 (sequence to rotate).
         * @param right Sequence 2 (sequence to fit).
         * @param substitution_scorer Match/mismatch (substitution) scorer.
         * @param gap_scorer Gap (indel) scorer.
         * @param freeride_scorer Freeride scorer.
         * @param max_path_weight_comparison_tolerance Tolerance used when testing for weight for equality. This may need to be non-zero
         *     when the type used for edge weights is a floating point type (must be finite). It helps mitigate floating point rounding
         *     errors when sequences are large / when scoring has large magnitude differences. The value this should be set to depends
         *     on multiple factors (e.g., which floating point type is used, expected graph size, expected magnitudes, etc..).
         * @return Rotational alignment of `down` vs `right` along with the score of that alignment.
         */
        template<
            widenable_to_size_t N_INDEX,
            weight WEIGHT,
            bool minimize_allocations = true
        >
        alignment_result auto align_strict(
            const sequence auto& down,
            const sequence auto& right,
            const scorer<
                N_INDEX,
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>,
                WEIGHT
            > auto& substitution_scorer,
            const scorer<
                N_INDEX,
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>,
                WEIGHT
            > auto& gap_scorer,
            const scorer<
                N_INDEX,
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>,
                WEIGHT
            > auto& freeride_scorer,
            const WEIGHT max_path_weight_comparison_tolerance
        ) {
            const auto& [leaf_down, leaf_right] {
                find_truncation_node<N_INDEX, WEIGHT, minimize_allocations>(
                    down,
                    right,
                    substitution_scorer,
                    gap_scorer,
                    freeride_scorer
                )
            };
            repeat_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(down)>
            > down_double { down, 2zu };
            using G = pairwise_fitting_alignment_graph<
                debug_mode,
                N_INDEX,
                WEIGHT,
                std::remove_cvref_t<decltype(down_double)>,
                std::remove_cvref_t<decltype(right)>,
                std::remove_cvref_t<decltype(substitution_scorer)>,
                std::remove_cvref_t<decltype(gap_scorer)>,
                std::remove_cvref_t<decltype(freeride_scorer)>
            >;
            G graph {
                down_double,
                right,
                substitution_scorer,
                gap_scorer,
                freeride_scorer
            };
            using PREFIX_G = prefix_sliceable_pairwise_alignment_graph<
                debug_mode,
                G
            >;
            using PREFIX_G_N = typename G::N;
            using PREFIX_G_E = typename G::E;
            using PREFIX_G_ED = typename G::ED;
            PREFIX_G_N prefix_graph_leaf {
                leaf_down,
                leaf_right
            };
            PREFIX_G prefix_graph {
                graph,
                prefix_graph_leaf
            };
            using CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
                debug_mode,
                PREFIX_G_N,
                PREFIX_G_E,
                PREFIX_G_ED,
                minimize_allocations
            >;
            backtracker<
                debug_mode,
                PREFIX_G,
                CONTAINER_CREATOR_PACK
            > backtracker_ {};
            auto&& [path, path_weight] {  // Use auto&& vs const auto&, otherwise move() below fails
                backtracker_.find_max_path(
                    prefix_graph,
                    max_path_weight_comparison_tolerance
                )
            };
            N_INDEX down_single_size {
                static_cast<N_INDEX>(down.size())
            };
            return std::make_pair(
                std::move(path)  // move() needed to ensure that transform operation below owns path
                    | std::views::transform(
                    // WARNING: Lambda capture of "graph" must be copy/move to avoid dangling ref.
                        [down_single_size](const PREFIX_G_E& e) {
                            // WARNING: DO NOT PASS IN prefix_graph AS A LAMBDA CAPTURE and use that to get sequence offsets.
                            //
                            //              std::optional<
                            //                 std::pair<
                            //                     std::optional<N_INDEX>,
                            //                     std::optional<N_INDEX>
                            //                 >
                            //              > offsets { prefix_graph.edge_to_element_offsets(edge) };
                            //
                            //          Even as a copy, prefix_graph  has a non-owning reference to the underlying graph that will be
                            //          destroyed once this function returns. Instead, the code logic has been replicated here.
                            using OPT_INDEX = std::optional<N_INDEX>;
                            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;
                            RET offsets {};
                            if (e.type == edge_type::FREE_RIDE) {
                                offsets = { std::nullopt };
                            }
                            constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu )};
                            const auto& [n1, n2] { e.inner_edge };
                            const auto& [n1_grid_down, n1_grid_right] { n1 };
                            const auto& [n2_grid_down, n2_grid_right] { n2 };
                            if (n1_grid_down + I1 == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                                offsets = { { { n1_grid_down }, { n1_grid_right } } };
                            } else if (n1_grid_down + I1 == n2_grid_down && n1_grid_right == n2_grid_right) {
                                offsets = { { { n1_grid_down }, std::nullopt } };
                            } else if (n1_grid_down == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                                offsets = { { std::nullopt, { n1_grid_right } } };
                            }
                            if (!offsets.has_value() || !std::get<0>(*offsets).has_value()) {
                                return offsets;
                            }
                            // Given that down is double'd - normalize the returned index so it only points to the original single down
                            // (what the user originally passed in to this align function).
                            N_INDEX down_double_idx { *std::get<0>(*offsets) };
                            if (down_double_idx >= down_single_size) {
                                down_double_idx -= down_single_size;
                                std::get<0>(*offsets) = std::optional<N_INDEX> { down_double_idx };
                            }
                            return offsets;
                        }
                    ),
                path_weight
            );
        }

    private:

        template<
            widenable_to_size_t N_INDEX,
            weight WEIGHT,
            bool minimize_allocations = true
        >
        std::pair<N_INDEX, N_INDEX> find_truncation_node(
            const sequence auto& down,
            const sequence auto& right,
            const scorer<
                N_INDEX,
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>,
                WEIGHT
            > auto& substitution_scorer,
            const scorer<
                N_INDEX,
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>,
                WEIGHT
            > auto& gap_scorer,
            const scorer<
                N_INDEX,
                std::remove_cvref_t<decltype(down[0])>,
                std::remove_cvref_t<decltype(right[0])>,
                WEIGHT
            > auto& freeride_scorer
        ) {
            repeat_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(down)>
            > down_double { down, 2zu };
            N_INDEX down_double_size {
                static_cast<N_INDEX>(down_double.size())
            };
            using G = pairwise_fitting_alignment_graph<
                debug_mode,
                N_INDEX,
                WEIGHT,
                std::remove_cvref_t<decltype(down_double)>,
                std::remove_cvref_t<decltype(right)>,
                std::remove_cvref_t<decltype(substitution_scorer)>,
                std::remove_cvref_t<decltype(gap_scorer)>,
                std::remove_cvref_t<decltype(freeride_scorer)>
            >;
            G graph {
                down_double,
                right,
                substitution_scorer,
                gap_scorer,
                freeride_scorer
            };
            using FORWARD_WALKER_CONTAINER_CREATOR_PACK = forward_walker_heap_container_creator_pack<
                debug_mode,
                typename G::N,
                typename G::E,
                typename G::ED,
                minimize_allocations
            >;
            auto fw {
                forward_walker<debug_mode, G, FORWARD_WALKER_CONTAINER_CREATOR_PACK>::create_and_initialize(
                    graph,
                    down_double_size,
                    WEIGHT {}
                )
            };
            typename G::N leaf_node { graph.get_leaf_node() };
            typename G::E final_edge {
                *fw.find(leaf_node).backtracking_edge
            };
            if (!graph.edge_to_element_offsets(final_edge).has_value()) {  // Missing means free ride, we want to terminate at its source
                leaf_node = graph.get_edge_from(final_edge);
            }
            const auto& [leaf_down, leaf_right, leaf_depth] {
                graph.node_to_grid_offset(leaf_node)
            };
            return std::pair<N_INDEX, N_INDEX> { leaf_down, leaf_right };
        }
    };

}

#endif //OFFBYNULL_ALIGNER_ALIGNERS_ROTATIONAL_SLICED_SUBDIVISION_HEAP_ALIGNER_H