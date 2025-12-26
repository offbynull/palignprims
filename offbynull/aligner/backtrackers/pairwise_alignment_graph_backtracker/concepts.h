#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H

#include "offbynull/concepts.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include <type_traits>
#include <concepts>
#include <utility>
#include <ranges>
#include <tuple>

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::concepts {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;

    /**
     * Concept that's satisfied if `T` has the traits of a path through a pairwise alignment graph.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept alignment_path =
        unqualified_object_type<T>
        && std::ranges::bidirectional_range<T>
        && requires(T t) {
            requires backtrackable_edge<std::remove_cvref_t<decltype(*t.begin())>>;
        };

    /**
     * Concept that's satisfied if `T` has the traits of a `std::pair<alignment_path, ED>`, where the first element is the
     * pairwise sequence alignment and the second element is the score of the pairwise sequence alignment.
     *
     * @tparam T Type to check.
     * @tparam ED Graph edge data type (edge weight).
     */
    template<typename T, typename ED>
    concept backtracking_result =
        unqualified_object_type<T>
        && weight<ED>
        && requires(T t) {
            requires alignment_path<std::remove_cvref_t<decltype(std::get<0>(t))>>;
            requires std::same_as<std::remove_cvref_t<decltype(std::get<1>(t))>, ED>;
        };

    /**
     * Concept that's satisfied if `T` has the traits of a `std::pair<alignment_path, weight>`, where the first element is the
     * pairwise sequence alignment and the second element is the score of the pairwise sequence alignment.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept backtracking_result_without_explicit_weight =
        unqualified_object_type<T>
        && requires(T t) {
            requires alignment_path<std::remove_cvref_t<decltype(std::get<0>(t))>>;
            requires weight<std::remove_cvref_t<decltype(std::get<1>(t))>>;
        };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H