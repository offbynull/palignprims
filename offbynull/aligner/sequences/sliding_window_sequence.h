#ifndef OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <array>
#include <vector>
#include <type_traits>

namespace offbynull::aligner::sequences::sliding_window_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::random_access_range_of_type;




    template<
        typename T,
        typename E
    >
    concept sliding_window_sequence_container_creator_pack =
        requires(const T t, std::size_t reserve_len) {
            { t.create_result_container(reserve_len) } -> random_access_range_of_type<E>;
        };

    template<
        bool debug_mode,
        typename E
    >
    struct sliding_window_sequence_heap_container_creator_pack {
        std::vector<E> create_result_container(std::size_t reserve_len) const {
            return std::vector<E>(reserve_len);
        }
    };

    template<
        bool debug_mode,
        typename E,
        std::size_t chunk_len
    >
    struct sliding_window_sequence_stack_container_creator_pack {
        std::array<E, chunk_len> create_result_container(std::size_t reserve_len) const {
            if constexpr (debug_mode) {
                if (chunk_len != reserve_len) {
                    throw std::runtime_error("Bad element count");
                }
            }
            return std::array<E, chunk_len> {};
        }
    };





    template<
        bool debug_mode,
        sequence SEQ,
        sliding_window_sequence_container_creator_pack<std::decay_t<decltype(std::declval<SEQ>()[0zu])>> CONTAINER_CREATOR_PACK = sliding_window_sequence_heap_container_creator_pack<debug_mode, std::decay_t<decltype(std::declval<SEQ>()[0zu])>>
    >
    class sliding_window_sequence {
    private:
        const SEQ& seq;
        const std::size_t window_length;

        using RESULT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_result_container(0zu));

        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        sliding_window_sequence(
            const SEQ& seq_,
            std::size_t window_length_,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : seq { seq_ }
        , window_length { window_length_ }
        , container_creator_pack { container_creator_pack_ } {
            if constexpr (debug_mode) {
                if (seq.size() < window_length) {
                    throw std::runtime_error("Window length too large");
                }
            }
        }

        RESULT_CONTAINER operator[](std::size_t index) const {
            RESULT_CONTAINER ret { container_creator_pack.create_result_container(window_length) };
            for (std::size_t i { 0zu }; i < window_length; i++) {
                ret[i] = seq[index + i];
            }
            return ret;
        }

        std::size_t size() const {
            return seq.size() - window_length + 1zu;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H
