#ifndef OFFBYNULL_ALIGNER_ALIGNERS_CONCEPTS_H
#define OFFBYNULL_ALIGNER_ALIGNERS_CONCEPTS_H

#include "offbynull/concepts.h"
#include "offbynull/aligner/concepts.h"
#include <type_traits>
#include <utility>
#include <optional>
#include <ranges>
#include <tuple>

namespace offbynull::aligner::aligners::concepts {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::aligner::concepts::weight;

    /**
     * Concept that's satisfied if `T` has the traits of a pairwise sequence alignment, where elements within each sequence are referenced
     * by index. The elements within the range should match the traits of ...
     *
     * ```
     * std::optional<  // Sequence index pair, where std::nullopt should be ignored (freeride)
     *     std::pair<
     *         std::optional<widenable_to_size_t>,  // Index within down sequence, where std::nullopt means gap (indel)
     *         std::optional<widenable_to_size_t>   // Index within right sequence, where std::nullopt means gap (indel)
     *     >
     * >
     * ```
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept bidirectional_range_of_alignment_index_pairs =
        unqualified_object_type<T>
        && std::ranges::bidirectional_range<T>
        && requires(T t) {
            requires widenable_to_size_t<std::remove_cvref_t<decltype(*std::get<0>(*(*t.begin())))>>;
            requires widenable_to_size_t<std::remove_cvref_t<decltype(*std::get<1>(*(*t.begin())))>>;
        };

    /**
     * Concept that's satisfied if `T` is a reference to a
     * @ref offbynull::aligner::aligners::concepts::bidirectional_range_of_alignment_index_pairs.
     */
    template<typename T>
    concept bidirectional_range_of_alignment_index_pairs_ref = std::is_reference_v<T>
        && bidirectional_range_of_alignment_index_pairs<std::remove_cvref_t<T>>;

    /**
     * Concept that's satisfied if `T` has the traits of a `std::pair<bidirectional_range_of_alignment_index_pairs, widenable_to_size_t>`,
     * where the first element is the pairwise sequence alignment and the second element is the score of the pairwise sequence alignment.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept alignment_result =
        unqualified_object_type<T>
        && requires(T t) {
            requires bidirectional_range_of_alignment_index_pairs<std::remove_cvref_t<decltype(std::get<0>(t))>>;
            requires weight<std::remove_cvref_t<decltype(std::get<1>(t))>>;
        };
}

#endif //OFFBYNULL_ALIGNER_ALIGNERS_CONCEPTS_H