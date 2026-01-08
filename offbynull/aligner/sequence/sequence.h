#ifndef OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H

#include <concepts>
#include <utility>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <string>
#include "offbynull/concepts.h"

namespace offbynull::aligner::sequence::sequence {
    using offbynull::concepts::convertible_to_unqualified_object_type;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Concept that's satisfied if `T` has the traits of a sequence. A sequence is an ordered collection that's randomly accessible.
     *
     * * Must allow access to the total number of elements it contains via the `size()` function.
     * * Must allow access to its elements via the `[]` operator.
     *   * `[]` operator's return type must be copyable and [regular](https://en.cppreference.com/w/cpp/concepts/regular).
     *
     * Common STL types such as `std::vector` and `std::string` satisfy this concept.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept sequence =
        unqualified_object_type<T>
        && requires(const T t) {
            { t[0zu] } -> convertible_to_unqualified_object_type;  // unqualified object type or convertible to one (copy constructor)
            { t.size() } -> widenable_to_size_t;
        }
        && std::regular<
            std::remove_cvref_t<
                decltype(std::declval<T>()[0zu])
            >
        >;

    /**
     * Unimplemented @ref offbynull::aligner::sequence::sequence::sequence, intended for documentation.
     */
    struct unimplemented_sequence {
        /**
         * Get element at index `index`.
         *
         * @param index Index of element.
         * @return Value at `index`.
         */
         decltype(auto) operator[](std::size_t index) const;

        /**
         * Get number of elements.
         *
         * @return Number of elements.
         */
        std::size_t size() const;
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H
