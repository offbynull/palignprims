#ifndef OFFBYNULL_ALIGNER_SCORER_SCORER_H
#define OFFBYNULL_ALIGNER_SCORER_SCORER_H

#include <concepts>
#include <optional>
#include <functional>
#include <utility>
#include <cstddef>
#include <type_traits>
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorer::scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Concept that's satisfied if `T` has the traits of a scorer. A scorer is a callable that scores a pairwise alignment graph's edge.
     *
     * @tparam T Type to check.
     * @tparam SEQ_INDEX_ Pairwise alignment graph's sequence indexer type.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     * @tparam WEIGHT_ Pairwise alignment graph's edge data type (edge's weight).
     */
    template<typename T, typename SEQ_INDEX_, typename DOWN_ELEM, typename RIGHT_ELEM, typename WEIGHT_>
    concept scorer =
        unqualified_object_type<T>
        && widenable_to_size_t<SEQ_INDEX_> && std::is_same_v<SEQ_INDEX_, typename T::SEQ_INDEX>  // Ensure nested type exists and is same
        && weight<WEIGHT_> && std::is_same_v<WEIGHT_, typename T::WEIGHT>  // Ensure nested type exists and is same
        && requires(
            const T t,
            const std::optional<
                std::pair<
                    SEQ_INDEX_,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem_ref_opt,
            const std::optional<
                std::pair<
                    SEQ_INDEX_,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem_ref_opt
        ) {
            { t(down_elem_ref_opt, right_elem_ref_opt) } -> std::same_as<WEIGHT_>;
        }
        && unqualified_object_type<DOWN_ELEM>
        && unqualified_object_type<RIGHT_ELEM>
        && weight<WEIGHT_>;

    /**
     * Equivalent to @ref offbynull::aligner::scorer::scorer::scorer, but doesn't enforce an exact type for the callable's return value.
     *
     * @tparam T Type to check.
     * @tparam SEQ_INDEX_ Pairwise alignment graph's sequence indexer type.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     */
    template<typename T, typename SEQ_INDEX_, typename DOWN_ELEM, typename RIGHT_ELEM>
    concept scorer_without_explicit_weight =
        unqualified_object_type<T>
        && widenable_to_size_t<SEQ_INDEX_> && std::is_same_v<SEQ_INDEX_, typename T::SEQ_INDEX>  // Ensure nested type exists and is same
        && weight<typename T::WEIGHT>
        && requires(
            const T t,
            const std::optional<
                std::pair<
                    SEQ_INDEX_,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem_ref_opt,
            const std::optional<
                std::pair<
                    SEQ_INDEX_,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem_ref_opt
        ) {
            { t(down_elem_ref_opt, right_elem_ref_opt) } -> std::same_as<typename T::WEIGHT>;
        }
        && unqualified_object_type<DOWN_ELEM>
        && unqualified_object_type<RIGHT_ELEM>;


    /**
     * Equivalent to @ref offbynull::aligner::scorer::scorer::scorer, but doesn't enforce an exact type for the callable's return value and
     * doesn't enforce and exact type for callable's sequence index parameters.
     *
     * @tparam T Type to check.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     */
    template<typename T, typename DOWN_ELEM, typename RIGHT_ELEM>
    concept scorer_without_explicit_weight_or_seq_index =
        unqualified_object_type<T>
        && widenable_to_size_t<typename T::SEQ_INDEX>
        && weight<typename T::WEIGHT>
        && requires(
            const T t,
            const std::optional<
                std::pair<
                    typename T::SEQ_INDEX,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem_ref_opt,
            const std::optional<
                std::pair<
                    typename T::SEQ_INDEX,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem_ref_opt
        ) {
            {t(down_elem_ref_opt, right_elem_ref_opt) } -> std::same_as<typename T::WEIGHT>;
        }
    && unqualified_object_type<DOWN_ELEM>
    && unqualified_object_type<RIGHT_ELEM>;

    /**
     * Unimplemented @ref offbynull::aligner::scorer::scorer::scorer, intended for documentation.
     *
     * @tparam SEQ_INDEX_ Pairwise alignment graph's sequence indexer type.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     */
    template<typename SEQ_INDEX_, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT_>
    requires requires (const DOWN_ELEM down_elem, const RIGHT_ELEM right_elem) {
        { down_elem == right_elem } -> std::same_as<bool>;
    }
    struct unimplemented_scorer {
        /**  Pairwise alignment graph's edge data type (edge's weight). */
        using WEIGHT = WEIGHT_;
        /** Sequence indexer type. */
        using SEQ_INDEX = SEQ_INDEX_;

        /**
         * Score edge.
         *
         *  * When `edge` is a freeride, both `down_elem` and `right_elem` are unassigned.
         *  * When `edge` is an indel, either `down_elem` or `right_elem` is assigned.
         *  * When `edge` is a substitution, both `down_elem` and `right_elem` are assigned.
         *
         * @param down_elem Downward sequence's element assigned to `edge` (`std::nullopt` if unassigned).
         * @param right_elem Rightward sequence's element assigned to `edge`  (`std::nullopt` if unassigned).
         * @return Score for `edge` (edge weight).
         */
        WEIGHT_ operator()(
            const std::optional<
                std::pair<
                    SEQ_INDEX_,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem,
            const std::optional<
                std::pair<
                    SEQ_INDEX_,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem
        ) const;
    };
    static_assert(
        scorer_without_explicit_weight<
            unimplemented_scorer<std::size_t, char, char, int>,
            std::size_t,
            char,
            char
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORER_SCORER_H
