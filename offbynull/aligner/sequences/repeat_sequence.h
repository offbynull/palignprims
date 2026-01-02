#ifndef OFFBYNULL_ALIGNER_SEQUENCES_REPEAT_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_REPEAT_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/utils.h"
#include <cstddef>
#include <type_traits>
#include <stdexcept>

namespace offbynull::aligner::sequences::repeat_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::utils::check_multiplication_nonoverflow_throwable;

    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that wraps an underlying
     * @ref offbynull::aligner::sequence::sequence::sequence and repeats it a certain number of times.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ Type of underlying @ref offbynull::aligner::sequence::sequence::sequence.
     */
    template<bool debug_mode, sequence SEQ>
    class repeat_sequence {
    private:
        const SEQ& seq;
        const std::size_t count;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::repeat_sequence::repeat_sequence instance.
         *
         * @param seq_ Underlying sequence.
         * @param count_ Number of times to repeat `seq_`.
         */
        repeat_sequence(
            const SEQ& seq_,
            std::size_t count_
        )
        : seq { seq_ }
        , count { count_ } {
            if constexpr (debug_mode) {
                check_multiplication_nonoverflow_throwable<std::size_t>(seq_.size(), count_);
            }
        }

        /**
         * @copydoc offbynull::aligner::sequence::sequence::unimplemented_sequence::operator[]
         */
        decltype(seq[0zu]) operator[](std::size_t index) const {
            if constexpr (debug_mode) {
                if (index >= seq.size() * count) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            std::size_t real_index { index % seq.size() };
            return seq[real_index];
        }

        /**
         * @copydoc offbynull::aligner::sequence::sequence::unimplemented_sequence::size()
         */
        std::size_t size() const {
            return seq.size() * count;
        }
    };

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::repeat_sequence::repeat_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seq Underlying sequence.
     * @param count Number of times to repeat `seq`.
     * @return Newly created @ref offbynull::aligner::sequences::repeat_sequence::repeat_sequence instance.
     */
    template<bool debug_mode>
    auto create_repeat_sequence(
        const sequence auto& seq,
        std::size_t count
    ) -> repeat_sequence<debug_mode, std::remove_cvref_t<decltype(seq)>> {
        return repeat_sequence<debug_mode, std::remove_cvref_t<decltype(seq)>> { seq, count };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_REPEAT_SEQUENCE_H
