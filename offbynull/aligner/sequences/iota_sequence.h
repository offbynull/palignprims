#ifndef OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H

#include "offbynull/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <stdexcept>
#include <limits>
#include <type_traits>

namespace offbynull::aligner::sequences::iota_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
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
                    throw std::runtime_error { "value exceeds bound" };
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

    template<bool debug_mode>
    auto create_iota_sequence(
        const widenable_to_size_t auto offset,
        const std::remove_cvref_t<decltype(offset)> bound
    ) {
        return iota_sequence<debug_mode, std::remove_cvref_t<decltype(offset)>> { offset, bound };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H
