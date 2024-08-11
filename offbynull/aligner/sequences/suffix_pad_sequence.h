#ifndef OFFBYNULL_ALIGNER_SEQUENCES_PREFIX_PAD_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_PREFIX_PAD_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/utils.h"
#include <cstddef>
#include <stdexcept>

namespace offbynull::aligner::sequences::suffix_pad_sequence {
    using offbynull::aligner::sequence::sequence::sequence;

    template<bool debug_mode, sequence SEQ>
    class suffix_pad_sequence {
    private:
        using SEQ_ELEM_RET_TYPE = decltype(std::declval<SEQ>()[0zu]);
        using SEQ_ELEM = std::remove_reference_t<SEQ_ELEM_RET_TYPE>;

        const SEQ& seq;
        const SEQ_ELEM pad_value;
        const std::size_t pad_count;

    public:
        suffix_pad_sequence(
            const SEQ& seq_,
            SEQ_ELEM pad_value_,
            std::size_t pad_count_
        )
        : seq { seq_ }
        , pad_value { pad_value_ }
        , pad_count { pad_count_ } {}

        SEQ_ELEM operator[](std::size_t index) const {
            if (index >= seq.size()) {
                return pad_value;
            }
            return seq[index];
        }

        std::size_t size() const {
            return seq.size() + pad_count;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_PREFIX_PAD_SEQUENCE_H
