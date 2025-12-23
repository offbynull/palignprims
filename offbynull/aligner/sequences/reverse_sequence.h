#ifndef OFFBYNULL_ALIGNER_SEQUENCES_REVERSE_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_REVERSE_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <type_traits>

namespace offbynull::aligner::sequences::reverse_sequence {
    using offbynull::aligner::sequence::sequence::sequence;

    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that wraps an underlying
     * @ref offbynull::aligner::sequence::sequence::sequence and reverses it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ Type of underlying @ref offbynull::aligner::sequence::sequence::sequence.
     */
    template<bool debug_mode, sequence SEQ>
    class reverse_sequence {
    private:
        const SEQ& seq;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::reverse_sequence::reverse_sequence instance.
         *
         * @param seq_ Underlying sequence.
         */
        reverse_sequence(const SEQ& seq_)
        : seq { seq_ } {}

        /**
         * @copydoc offbynull::aligner::sequence::sequence::unimplemented_sequence::operator[]
         */
        decltype(seq[0zu]) operator[](std::size_t index) const {
            return seq[seq.size() - index - 1zu];
        }

        /**
         * @copydoc offbynull::aligner::sequence::sequence::unimplemented_sequence::size()
         */
        std::size_t size() const {
            return seq.size();
        }
    };

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::reverse_sequence::reverse_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seq Underlying sequence.
     * @return Newly created @ref offbynull::aligner::sequences::reverse_sequence::reverse_sequence instance.
     */
    template<bool debug_mode>
    auto create_reverse_sequence(
        const sequence auto& seq
    ) -> reverse_sequence<debug_mode, std::remove_cvref_t<decltype(seq)>> {
        return reverse_sequence<debug_mode, std::remove_cvref_t<decltype(seq)>> { seq };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_REVERSE_SEQUENCE_H
