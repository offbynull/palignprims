#ifndef OFFBYNULL_ALIGNER_SEQUENCES_MIDDLE_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_MIDDLE_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <stdexcept>

namespace offbynull::aligner::sequences::substring_sequence {
    using offbynull::aligner::sequence::sequence::sequence;

    template<bool error_check, sequence SEQ>
    class substring_sequence {
    private:
        const SEQ& seq;
        const std::size_t offset;
        const std::size_t length;

    public:
        substring_sequence(const SEQ& seq_, std::size_t offset_, std::size_t length_)
        : seq { seq_ }
        , offset { offset_ }
        , length { length_ } {
            if constexpr (error_check) {
                if (seq.length() - offset < length_) {
                    throw std::runtime_error("Out of bounds length");
                }
            }
        }

        decltype(seq[0zu]) operator[](std::size_t index) const {
            return seq[index + offset];
        }

        std::size_t size() const {
            return length;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_MIDDLE_SEQUENCE_H
