#ifndef OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H

#include "offbynull/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <stdexcept>
#include <limits>
#include <type_traits>


/**
 * @ref offbynull::aligner::sequence::sequence::sequence backed by an integer range.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::aligner::sequences::iota_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that returns incrementally increasing integers (values within an integer
     * range).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam I Type of integer.
     */
    template<bool debug_mode, widenable_to_size_t I>
    class iota_sequence {
    private:
        const I offset;
        const I bound;

    public:
        /**
         * Construct an offbynull::aligner::sequences::iota_sequence::iota_sequence instance where the range begins at `offset_` and ends at
         * `std::numeric_limits<I>::max()` (inclusive).
         *
         * @param offset_ Range start.
         */
        iota_sequence(I offset_)
        : iota_sequence { offset_, std::numeric_limits<I>::max() } {}

        /**
         * Construct an offbynull::aligner::sequences::iota_sequence::iota_sequence instance where the range begins at `offset_` and ends at
         * `bound_` (inclusive).
         *
         * @param offset_ Range start.
         * @param bound_ Range end (inclusive).
         * @throws std::runtime_error If `debug_mode && offset_ > bound_`.
         */
        iota_sequence(I offset_, I bound_)
        : offset { offset_ }
        , bound { bound_ } {
            if constexpr (debug_mode) {
                if (offset > bound) {
                    throw std::runtime_error { "Value exceeds bound" };
                }
            }
        }

        /**
         * Get element at index `index`.
         *
         * @param index Index of element.
         * @return Value at `index`.
         */
        I operator[](std::size_t index) const {
            return static_cast<I>(index + offset);
        }

        /**
         * Get number of elements.
         *
         * @return Number of elements.
         */
        std::size_t size() const {
            return bound - offset;
        }
    };

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::iota_sequence::iota_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @return Newly created @ref offbynull::aligner::sequences::iota_sequence::iota_sequence instance.
     * @throws std::runtime_error If `debug_mode && offset_ > bound_`.
     */
    template<bool debug_mode>
    auto create_iota_sequence(
        const widenable_to_size_t auto offset,
        const std::remove_cvref_t<decltype(offset)> bound
    ) {
        return iota_sequence<debug_mode, std::remove_cvref_t<decltype(offset)>> { offset, bound };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_IOTA_SEQUENCE_H
