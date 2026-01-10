#ifndef OFFBYNULL_ALIGNER_ALIGNERS_LOCAL_DYNAMIC_PROGRAMMING_STACK_ALIGNER_H
#define OFFBYNULL_ALIGNER_ALIGNERS_LOCAL_DYNAMIC_PROGRAMMING_STACK_ALIGNER_H

#include <utility>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <ranges>
#include <stdexcept>
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/widening_scorer.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/aligners/local_dynamic_programming_alignment_type_parameterizer.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/aligners/internal_utils.h"

namespace offbynull::aligner::aligners::local_dynamic_programming_stack_aligner {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::numeric;
    using offbynull::utils::wider_numeric;
    using offbynull::utils::compile_time_constant;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight_or_seq_index;
    using offbynull::aligner::scorers::widening_scorer::widening_scorer;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_stack_container_creator_pack
        ::backtracker_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::aligners::local_dynamic_programming_alignment_type_parameterizer
        ::local_dynamic_programming_alignment_type_parameterizer;
    using offbynull::aligner::aligners::concepts::alignment_result;
    using offbynull::aligner::aligners::internal_utils::validate_align_and_extract_widest_weight;

    /**
     * Local pairwise aligner using dynamic programming algorithm (see
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker) and stack allocation.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     */
    template<bool debug_mode>
    class local_dynamic_programming_stack_aligner {
    public:

        /**
         * Align two sequences.
         *
         * @tparam max_down_seq_size Maximum allowed length for downward sequence.
         * @tparam max_right_seq_size Maximum allowed length for rightward sequence.
         * @tparam N_INDEX Alignment graph node coordinate type (must be wide enough to hold `std::max(down.size(), right.size()) + 1zu`).
         * @param down Sequence 1.
         * @param right Sequence 2.
         * @param substitution_scorer Match/mismatch (substitution) scorer.
         * @param gap_scorer Gap (indel) scorer.
         * @param freeride_scorer Freeride scorer.
         * @return Local alignment of `down` vs `right` along with the score of that alignment.
         */
        template<
            std::size_t max_down_seq_size,
            std::size_t max_right_seq_size,
            widenable_to_size_t N_INDEX = std::size_t
        >
        auto align(
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

            return align_strict<N_INDEX, WEIGHT, max_down_seq_size, max_right_seq_size>(
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
         * @tparam max_down_seq_size Maximum allowed length for downward sequence.
         * @tparam max_right_seq_size Maximum allowed length for rightward sequence.
         * @tparam WEIGHT Alignment graph edge weight type.
         * @tparam N_INDEX Alignment graph node coordinate type (must be wide enough to hold `std::max(down.size(), right.size()) + 1zu`).
         * @param down Sequence 1.
         * @param right Sequence 2.
         * @param substitution_scorer Match/mismatch (substitution) scorer.
         * @param gap_scorer Gap (indel) scorer.
         * @param freeride_scorer Freeride scorer.
         * @return Local alignment of `down` vs `right` along with the score of that alignment.
         */
        template<
            widenable_to_size_t N_INDEX,
            weight WEIGHT,
            std::size_t max_down_seq_size,
            std::size_t max_right_seq_size
        >
        auto align_strict(
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
            if constexpr (debug_mode) {
                if (down.size() > max_down_seq_size || right.size() > max_right_seq_size) {
                    throw std::runtime_error { "Sequence length larger than expected (for stack allocation)" };
                }
            }
            using G = pairwise_local_alignment_graph<
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
            ;
            using CONTAINER_CREATOR_PACK = backtracker_stack_container_creator_pack<
                debug_mode,
                typename G::N,
                typename G::E,
                typename G::ED,
                typename local_dynamic_programming_alignment_type_parameterizer<N_INDEX>::PARENT_COUNT,
                typename local_dynamic_programming_alignment_type_parameterizer<N_INDEX>::SLOT_INDEX,
                G::axis_node_length(max_down_seq_size),  // max_down_seq_size + 1zu
                G::axis_node_length(max_right_seq_size),  // max_right_seq_size + 1zu
                G::grid_depth_cnt,  // 1 depth
                G::maximum_path_edge_count(max_down_seq_size, max_right_seq_size)
            >;
            backtracker<
                debug_mode,
                G,
                typename local_dynamic_programming_alignment_type_parameterizer<N_INDEX>::PARENT_COUNT,
                typename local_dynamic_programming_alignment_type_parameterizer<N_INDEX>::SLOT_INDEX,
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

#endif //OFFBYNULL_ALIGNER_ALIGNERS_LOCAL_DYNAMIC_PROGRAMMING_STACK_ALIGNER_H