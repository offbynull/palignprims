#ifndef OFFBYNULL_ALIGNER_SEQUENCES_CHUNKED_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_CHUNKED_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/helpers/container_creators.h"
#include <cstddef>
#include <array>
#include <type_traits>
#include <stdexcept>

namespace offbynull::aligner::sequences::chunked_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::small_vector_container_creator;

    template<bool error_check, sequence SEQ, std::size_t CHUNK_LENGTH>
    class compiletime_chunked_sequence {
    private:
        const SEQ& seq;

        using INNER_ELEM = std::decay_t<decltype(seq[0zu])>;

    public:
        compiletime_chunked_sequence(const SEQ& seq_)
        : seq { seq_ } {}

        std::array<INNER_ELEM, CHUNK_LENGTH> operator[](std::size_t index) const {
            std::size_t offset { index * CHUNK_LENGTH };
            std::array<INNER_ELEM, CHUNK_LENGTH> ret { };
            for (std::size_t i { 0zu }; i < CHUNK_LENGTH; i++) {
                ret[i] = seq[offset + i];
            }
            return ret;
        }

        std::size_t size() const {
            return seq.size() / CHUNK_LENGTH;
        }
    };



    template<
        bool error_check,
        sequence SEQ,
        container_creator CONTAINER_CREATOR = small_vector_container_creator<
            std::decay_t<decltype(std::declval<SEQ>()[0zu])>,
            12zu,
            false
        >
    >
    class runtime_chunked_sequence {
    private:
        const SEQ& seq;
        const std::size_t chunk_length;
        const CONTAINER_CREATOR container_creator;

        using INNER_ELEM = std::decay_t<decltype(seq[0zu])>;

    public:
        runtime_chunked_sequence(
            SEQ& seq_,
            std::size_t chunk_length_,
            CONTAINER_CREATOR container_creator_ = {}
        )
        : seq { seq_ }
        , chunk_length { chunk_length_ }
        , container_creator { container_creator_ } {}

        auto operator[](std::size_t index) const {
            std::size_t offset { index * chunk_length };
            auto ret { container_creator.create_objects({ chunk_length }) };
            for (std::size_t i { 0zu }; i < chunk_length; i++) {
                ret[i] = seq[offset + i];
            }
            return ret;
        }

        std::size_t size() const {
            return seq.size() / chunk_length;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_CHUNKED_SEQUENCE_H
