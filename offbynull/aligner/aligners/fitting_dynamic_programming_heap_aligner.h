#ifndef OFFBYNULL_ALIGNER_ALIGNERS_FITTING_DYNAMIC_PROGRAMMING_HEAP_ALIGNER_H
#define OFFBYNULL_ALIGNER_ALIGNERS_FITTING_DYNAMIC_PROGRAMMING_HEAP_ALIGNER_H

#include <utility>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <ranges>
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/widening_scorer.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/aligners/fitting_dynamic_programming_alignment_type_parameterizer.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/aligners/concepts.h"
#include "offbynull/aligner/aligners/internal_utils.h"

namespace offbynull::aligner::aligners::fitting_dynamic_programming_heap_aligner {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::numeric;
    using offbynull::utils::wider_numeric;
    using offbynull::utils::compile_time_constant;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight_or_seq_index;
    using offbynull::aligner::scorers::widening_scorer::widening_scorer;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_heap_container_creator_pack
        ::backtracker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::aligners::fitting_dynamic_programming_alignment_type_parameterizer
        ::fitting_dynamic_programming_alignment_type_parameterizer;
    using offbynull::aligner::aligners::concepts::alignment_result;
    using offbynull::aligner::aligners::internal_utils::validate_align_and_extract_widest_weight;

    /**
     * Fitting pairwise aligner using dynamic programming algorithm (see
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker) and heap allocation.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     */
    template<bool debug_mode>
    class fitting_dynamic_programming_heap_aligner {
    public:

        /**
         * Align two sequences.
         *
         * @tparam minimize_allocations `true` to force certain internal containers to reserve (preallocate) elements thereby
         *     removing/reducing the need for adhoc reallocations, `false` otherwise.
         * @tparam N_INDEX Alignment graph node coordinate type (must be wide enough to hold `std::max(down.size(), right.size()) + 1zu`).
         * @param down Sequence 1 (sequence to fit against).
         * @param right Sequence 2 (sequence to fit).
         * @param substitution_scorer Match/mismatch (substitution) scorer.
         * @param gap_scorer Gap (indel) scorer.
         * @param freeride_scorer Freeride scorer.
         * @return Fitting alignment of `down` vs `right` along with the score of that alignment.
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
            > auto& freeride_scorer
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
                widened_freeride_scorer
            );
        }

        /**
         * Align two sequences.
         *
         * @tparam minimize_allocations `true` to force certain internal containers to reserve (preallocate) elements thereby
         *     removing/reducing the need for adhoc reallocations, `false` otherwise.
         * @tparam WEIGHT Alignment graph edge weight type.
         * @tparam N_INDEX Alignment graph node coordinate type (must be wide enough to hold `std::max(down.size(), right.size()) + 1zu`).
         * @param down Sequence 1 (sequence to fit against).
         * @param right Sequence 2 (sequence to fit).
         * @param substitution_scorer Match/mismatch (substitution) scorer.
         * @param gap_scorer Gap (indel) scorer.
         * @param freeride_scorer Freeride scorer.
         * @return Fitting alignment of `down` vs `right` along with the score of that alignment.
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
            > auto& freeride_scorer
        ) {
            using G = pairwise_fitting_alignment_graph<
                debug_mode,
                N_INDEX,
                WEIGHT,
                std::remove_cvref_t<decltype(down)>,
                std::remove_cvref_t<decltype(right)>,
                std::remove_cvref_t<decltype(substitution_scorer)>,
                std::remove_cvref_t<decltype(gap_scorer)>,
                std::remove_cvref_t<decltype(freeride_scorer)>
            >;
            G graph {
                down,
                right,
                substitution_scorer,
                gap_scorer,
                freeride_scorer
            };
            using CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
                debug_mode,
                typename G::N,
                typename G::E,
                typename G::ED,
                typename fitting_dynamic_programming_alignment_type_parameterizer<N_INDEX>::PARENT_COUNT,
                typename fitting_dynamic_programming_alignment_type_parameterizer<N_INDEX>::SLOT_INDEX,
                minimize_allocations
            >;
            backtracker<
                debug_mode,
                G,
                typename fitting_dynamic_programming_alignment_type_parameterizer<N_INDEX>::PARENT_COUNT,
                typename fitting_dynamic_programming_alignment_type_parameterizer<N_INDEX>::SLOT_INDEX,
                CONTAINER_CREATOR_PACK
            > backtracker_ {};
            auto&& [path, path_weight] { backtracker_.find_max_path(graph) };  // Use auto&& vs const auto&, otherwise move() below fails
            return std::make_pair(
                std::move(path)  // move() needed to ensure that transform operation below owns path
                    | std::views::transform(
                        [graph](const typename G::E& edge) {  // WARNING: Lambda capture of "graph" must be copy/move to avoid dangling ref.
                            return graph.edge_to_element_offsets(edge);
                        }
                    ),
                path_weight
            );
        }
    };

}

#endif //OFFBYNULL_ALIGNER_ALIGNERS_FITTING_DYNAMIC_PROGRAMMING_HEAP_ALIGNER_H