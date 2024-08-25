#ifndef OFFBYNULL_ALIGNER_CONCEPTS_H
#define OFFBYNULL_ALIGNER_CONCEPTS_H

#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::aligner::concepts {
    using offbynull::concepts::unqualified_value_type;

    template<typename T>
    concept weight =
        unqualified_value_type<T>
        && requires(T t) {
            { t + t } -> std::same_as<T>;
            { t < t } -> std::same_as<bool>;
        };
}

#endif //OFFBYNULL_ALIGNER_CONCEPTS_H
