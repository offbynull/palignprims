#ifndef OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H

#include "offbynull/concepts.h"
#include <cstddef>
#include <stdexcept>
#include <limits>

namespace offbynull::aligner::sequences::iota_sequence {
    using offbynull::concepts::widenable_to_size_t;

    template<bool debug_mode, widenable_to_size_t I>
    class iota_sequence {
    private:
        const I offset;
        const I bound;

    public:
        iota_sequence(I offset_)
        : iota_sequence { offset_, std::numeric_limits<I>::max() } {}

        iota_sequence(I offset_, I bound_)
        : offset { offset_ }
        , bound { bound_ } {
            if constexpr (debug_mode) {
                if (offset > bound) {
                    throw std::runtime_error("value exceeds bound");
                }
            }
        }

        I operator[](std::size_t index) const {
            return static_cast<I>(index + offset);
        }

        std::size_t size() const {
            return bound - offset;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H
