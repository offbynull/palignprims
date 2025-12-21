#ifndef OFFBYNULL_ALIGNER_SCORER_SCORER_H
#define OFFBYNULL_ALIGNER_SCORER_SCORER_H

#include <concepts>
#include <optional>
#include <functional>
#include <utility>
#include <cstddef>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorer::scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Concept that's satisfied if `T` has the traits of a scorer. A scorer is a callable that scores an alignment graph's edge.
     *
     * @tparam T Type to check.
     * @tparam E Alignment graph's edge identifier type.
     * @tparam SEQ_INDEX Alignment graph's sequence indexer type.
     * @tparam DOWN_ELEM Alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Alignment graph's rightward sequence element type.
     * @tparam WEIGHT Alignment graph's edge weight type.
     */
    template<typename T, typename E, typename SEQ_INDEX, typename DOWN_ELEM, typename RIGHT_ELEM, typename WEIGHT>
    concept scorer =
        unqualified_object_type<T>
        && widenable_to_size_t<SEQ_INDEX>
        && requires(
            T t,
            const E& edge,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem_ref_opt,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem_ref_opt
        ) {
            { t(edge, down_elem_ref_opt, right_elem_ref_opt) } -> std::same_as<WEIGHT>;  // TODO: Update to include sequence index
        }
        && unqualified_object_type<DOWN_ELEM>
        && unqualified_object_type<RIGHT_ELEM>
        && weight<WEIGHT>;

    /**
     * Equivalent to @ref offbynull::aligner::scorer::scorer::scorer, but doesn't enforce an exact type for the callable's return value.
     *
     * @tparam T Type to check.
     * @tparam E Alignment graph's edge identifier type.
     * @tparam SEQ_INDEX Alignment graph's sequence indexer type.
     * @tparam DOWN_ELEM Alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Alignment graph's rightward sequence element type.
     */
    template<typename T, typename E, typename SEQ_INDEX, typename DOWN_ELEM, typename RIGHT_ELEM>
    concept scorer_without_explicit_weight =
        unqualified_object_type<T>
        && widenable_to_size_t<SEQ_INDEX>
        && requires(
            T t,
            const E& edge,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem_ref_opt,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem_ref_opt
        ) {
            { t(edge, down_elem_ref_opt, right_elem_ref_opt) } -> weight;
        }
        && unqualified_object_type<DOWN_ELEM>
        && unqualified_object_type<RIGHT_ELEM>;


    /**
     * Unimplemented @ref offbynull::aligner::scorer::scorer::scorer, intended for documentation.
     *
     * @tparam SEQ_INDEX Alignment graph's sequence indexer type.
     * @tparam DOWN_ELEM Alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Alignment graph's rightward sequence element type.
     */
    template<typename SEQ_INDEX, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    requires requires (const DOWN_ELEM down_elem, const RIGHT_ELEM right_elem) {
        { down_elem == right_elem } -> std::same_as<bool>;
    }
    struct unimplemented_scorer {
        /**
         * Score edge.
         *
         *  * When `edge` is a freeride, both `down_elem` and `right_elem` are unassigned.
         *  * When `edge` is an indel, either `down_elem` or `right_elem` is assigned.
         *  * When `edge` is a substitution, both `down_elem` and `right_elem` are assigned.
         *
         * @param edge Edge.
         * @param down_elem Downward sequence's element assigned to `edge` (`std::nullopt` if unassigned).
         * @param right_elem Rightward sequence's element assigned to `edge`  (`std::nullopt` if unassigned).
         * @return Score for edge (edge weight).
         */
        WEIGHT operator()(
            const auto& edge,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem
        ) const;
    };
    static_assert(
        scorer_without_explicit_weight<
            unimplemented_scorer<std::size_t, char, char, int>,
            int,
            std::size_t,
            char,
            char
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORER_SCORER_H
