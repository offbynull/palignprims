#ifndef OFFBYNULL_ALIGNER_SEQUENCES_CHUNKED_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_CHUNKED_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <array>
#include <vector>
#include <type_traits>
#include <utility>
#include <stdexcept>

namespace offbynull::aligner::sequences::chunked_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_value_type;



    template<
        typename T,
        typename E
    >
    concept chunked_sequence_container_creator_pack =
        unqualified_value_type<T>
        && requires(const T t, std::size_t reserve_len) {
            { t.create_result_container(reserve_len) } -> random_access_range_of_type<E>;
        };

    template<
        bool debug_mode,
        typename E
    >
    struct chunked_sequence_heap_container_creator_pack {
        std::vector<E> create_result_container(std::size_t reserve_len) const {
            return std::vector<E>(reserve_len);
        }
    };

    template<
        bool debug_mode,
        typename E,
        std::size_t chunk_len
    >
    struct chunked_sequence_stack_container_creator_pack {
        std::array<E, chunk_len> create_result_container(std::size_t reserve_len) const {
            if constexpr (debug_mode) {
                if (chunk_len != reserve_len) {
                    throw std::runtime_error("Bad element count");
                }
            }
            return {};
        }
    };





    template<
        bool debug_mode,
        sequence SEQ,
        chunked_sequence_container_creator_pack<std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>> CONTAINER_CREATOR_PACK =
            chunked_sequence_heap_container_creator_pack<debug_mode, std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>>
    >
    class chunked_sequence {
    private:
        const SEQ& seq;
        const std::size_t chunk_length;

        using RESULT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_result_container(0zu));

        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        chunked_sequence(
            const SEQ& seq_,
            std::size_t chunk_length_,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : seq { seq_ }
        , chunk_length { chunk_length_ }
        , container_creator_pack { container_creator_pack_ } {}

        RESULT_CONTAINER operator[](std::size_t index) const {
            std::size_t offset { index * chunk_length };
            RESULT_CONTAINER ret { container_creator_pack.create_result_container(chunk_length) };
            for (std::size_t i { 0zu }; i < chunk_length; i++) {
                ret[i] = seq[offset + i];
            }
            return ret;
        }

        std::size_t size() const {
            return seq.size() / chunk_length;
        }
    };

    template<bool debug_mode>
    sequence auto create_heap_chunked_sequence(const sequence auto& backing_sequence_, std::size_t chunk_length) {
        return
            chunked_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(backing_sequence_)>
            > { backing_sequence_, chunk_length };
    }

    template<bool debug_mode, std::size_t chunk_length>
    sequence auto create_stack_chunked_sequence(const sequence auto& backing_sequence_) {
        using ELEM = std::remove_cvref_t<decltype(backing_sequence_[0zu])>;
        return
             chunked_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(backing_sequence_)>,
                chunked_sequence_stack_container_creator_pack<debug_mode, ELEM, chunk_length>
            > { backing_sequence_, chunk_length };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_CHUNKED_SEQUENCE_H
