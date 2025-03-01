#ifndef OFFBYNULL_ALIGNER_SEQUENCES_SUBSTRING_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_SUBSTRING_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <stdexcept>
#include <type_traits>

/**
 * Clipped @ref offbynull::aligner::sequence::sequence::sequence.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::aligner::sequences::substring_sequence {
    using offbynull::aligner::sequence::sequence::sequence;

    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that wraps an underlying
     * @ref offbynull::aligner::sequence::sequence::sequence and clips the beginning and end, similar to clipping the beginning and end of a
     * string to get a substring.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ Type of underlying @ref offbynull::aligner::sequence::sequence::sequence.
     */
    template<bool debug_mode, sequence SEQ>
    class substring_sequence {
    private:
        const SEQ& seq;
        const std::size_t offset;
        const std::size_t length;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::substring_sequence::substring_sequence instance.
         *
         * @param seq_ Underlying sequence.
         * @param offset_ Index within `seq_` to start from.
         * @param length_ Number of elements within this sequence.
         * @throws std::runtime_error If `debug_mode && seq_.length() - offset_ < length_` (extends past end of `seq_`).
         */
        substring_sequence(const SEQ& seq_, std::size_t offset_, std::size_t length_)
        : seq { seq_ }
        , offset { offset_ }
        , length { length_ } {
            if constexpr (debug_mode) {
                if (seq.length() - offset < length_) {
                    throw std::runtime_error { "Length out of bounds" };
                }
            }
        }

        /**
         * Get element at index `index`.
         *
         * @param index Index of element.
         * @return Value at `index`.
         */
        decltype(seq[0zu]) operator[](std::size_t index) const {
            if constexpr (debug_mode) {
                if (index >= length) {
                    throw std::runtime_error { "Index out of bounds" };
                }
            }
            return seq[index + offset];
        }

        /**
         * Get number of elements.
         *
         * @return Number of elements.
         */
        std::size_t size() const {
            return length;
        }
    };

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::substring_sequence::substring_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seq Underlying sequence.
     * @param offset Index within `seq_` to start from.
     * @param length Number of elements within this sequence.
     * @return Newly created @ref offbynull::aligner::sequences::substring_sequence::substring_sequence instance.
     * @throws std::runtime_error If `debug_mode && seq.length() - offset < length` (extends past end of `seq`).
     */
    template<bool debug_mode>
    auto create_substring_sequence(
        const sequence auto& seq,
        const std::size_t offset,
        const std::size_t length
    ) {
        return substring_sequence<debug_mode, std::remove_cvref_t<decltype(seq)>> { seq, offset, length };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_SUBSTRING_SEQUENCE_H
