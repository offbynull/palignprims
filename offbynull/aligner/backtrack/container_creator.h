#ifndef OFFBYNULL_ALIGNER_BACKTRACK_CONTAINER_CREATOR_H
#define OFFBYNULL_ALIGNER_BACKTRACK_CONTAINER_CREATOR_H

#include <cstddef>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrack::container_creator {
    using offbynull::concepts::random_access_range_of_type;

    // You can use unimplemented types as requires params -- the compiler will check to see if it has the same traits
    template<typename T>
    struct unimplemented_input_iterator {
        using difference_type = std::ptrdiff_t;
        using value_type = T;

        T operator*() const;
        unimplemented_input_iterator& operator++();
        unimplemented_input_iterator operator++(int) { ++*this; }
    };

    template <typename T>
    concept container_creator =
        requires(T t, std::size_t size, std::optional<std::size_t> capacity, unimplemented_input_iterator<typename T::ELEM> it) {
            typename T::ELEM;
            { t.create_empty(capacity) } -> random_access_range_of_type<typename T::ELEM>;
            { t.create_objects(size) } -> random_access_range_of_type<typename T::ELEM>;
            { t.create_copy(it, it) } -> random_access_range_of_type<typename T::ELEM>;
        };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_CONTAINER_CREATOR_H
