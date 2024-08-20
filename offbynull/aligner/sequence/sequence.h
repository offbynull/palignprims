#ifndef OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H

#include <concepts>

namespace offbynull::aligner::sequence::sequence {
    template<typename T>
    concept decayable_type = !std::is_void_v<T> && std::is_convertible_v<T, std::decay_t<T>>;

    template<typename T>
    concept sequence =
        requires(T t) {
            { t[0zu] } -> decayable_type;  // Convertible to non-void type that decays (only needs copy constructor?)
            { t.size() } -> std::same_as<std::size_t>;
        } &&
        std::regular<
            std::decay_t<
                decltype(std::declval<T>()[0zu])
            >
        >;
}

#endif //OFFBYNULL_ALIGNER_SEQUENCE_SEQUENCE_H
