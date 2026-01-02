#ifndef OFFBYNULL_ALIGNER_CONCEPTS_H
#define OFFBYNULL_ALIGNER_CONCEPTS_H

#include <concepts>
#include <type_traits>
#include "offbynull/concepts.h"

namespace offbynull::aligner::concepts {
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `T` has the traits of an edge weight within an alignment graph.
     *
     * * Must be additive via the `+` operator.
     * * Must be comparable via the `<` operator.
     *
     * Common number types such as integers and floats satisfy this concept.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept weight =
        unqualified_object_type<T>
        && std::is_copy_constructible_v<T>
        && std::is_copy_assignable_v<T>
        && std::is_move_constructible_v<T>
        && std::is_move_assignable_v<T>
        && requires(const T t) {
            { t + t } -> std::convertible_to<T>;
            { t - t } -> std::convertible_to<T>;
            { t < t } -> std::same_as<bool>;
            { t <= t } -> std::same_as<bool>;
            { t > t } -> std::same_as<bool>;
            { t >= t } -> std::same_as<bool>;
            { t == t } -> std::same_as<bool>;
            { t != t } -> std::same_as<bool>;
        };
}

#endif //OFFBYNULL_ALIGNER_CONCEPTS_H
