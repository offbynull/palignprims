#ifndef OFFBYNULL_ALIGNER_SEQUENCES_REVERSED_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_REVERSED_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>

namespace offbynull::aligner::sequences::reversed_sequence {
    using offbynull::aligner::sequence::sequence::sequence;

    template<bool debug_mode, sequence SEQ>
    class reversed_sequence {
    private:
        const SEQ& seq;

    public:
        reversed_sequence(const SEQ& seq_)
        : seq { seq_ } { }

        decltype(seq[0zu]) operator[](std::size_t index) const {
            return seq[seq.size() - index - 1zu];
        }

        std::size_t size() const {
            return seq.size();
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_REVERSED_SEQUENCE_H
