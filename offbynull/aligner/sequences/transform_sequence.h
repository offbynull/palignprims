#ifndef OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H

#include "offbynull/concepts.h"
#include <cstddef>
#include <functional>

namespace offbynull::aligner::sequences::transform_sequence {
    using offbynull::concepts::widenable_to_size_t;

    template<bool error_check, typename ELEM>
    class transform_sequence {
    private:
        const std::size_t size_;
        const std::function<ELEM(std::size_t)> transformer;

    public:
        transform_sequence(std::size_t size__, std::function<ELEM(std::size_t)> transformer_)
        : size_ { size__ }
        , transformer { transformer_ } {}

        ELEM operator[](std::size_t index) const {
            return transformer(index);
        }

        std::size_t size() const {
            return size_;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H
