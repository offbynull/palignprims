#ifndef OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H

#include <concepts>
#include <utility>
#include <cstddef>
#include <type_traits>
#include "offbynull/concepts.h"

namespace offbynull::aligner::sequence::sequence {
    using offbynull::concepts::convertible_to_unqualified_value_type;

    template<typename T>
    concept sequence =
        requires(T t) {
            { t[0zu] } -> convertible_to_unqualified_value_type;  // Returns unqualified value type or convertable to one (copy constructor)
            { t.size() } -> std::same_as<std::size_t>;
        } &&
        std::regular<
            std::decay_t<
                decltype(std::declval<T>()[0zu])
            >
        >;
}

#endif //OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H
