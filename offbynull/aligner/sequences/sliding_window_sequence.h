#ifndef OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <array>
#include <vector>
#include <utility>
#include <type_traits>
#include <stdexcept>

namespace offbynull::aligner::sequences::sliding_window_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::random_access_sequence_container;
    using offbynull::concepts::unqualified_object_type;


    /**
     * Concept that's satisfied if `T` has the traits for creating the containers required by
     * @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence, referred to as a container creator pack.
     *
     * @tparam T Type to check.
     * @tparam E Type of element within sequence.
     */
    template<
        typename T,
        typename E
    >
    concept sliding_window_sequence_container_creator_pack =
        unqualified_object_type<T>
        && requires(const T t, std::size_t reserve_len) {
            { t.create_result_container(reserve_len) } -> random_access_sequence_container<E>;
        };

    /**
     * Container creator pack that allocates containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam E Type of element within sequence.
     */
    template<
        bool debug_mode,
        typename E
    >
    struct sliding_window_sequence_heap_container_creator_pack {
        std::vector<E> create_result_container(std::size_t reserve_len) const {
            return std::vector<E>(reserve_len);
        }
    };

    /**
     * Container creator pack that allocates containers on the stack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam E Type of element within sequence.
     * @tparam chunk_len Number of elements in each chunk.
     */
    template<
        bool debug_mode,
        typename E,
        std::size_t chunk_len
    >
    struct sliding_window_sequence_stack_container_creator_pack {
        std::array<E, chunk_len> create_result_container(std::size_t reserve_len) const {
            if constexpr (debug_mode) {
                if (chunk_len != reserve_len) {
                    throw std::runtime_error { "Bad element count" };
                }
            }
            return {};
        }
    };




    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that wraps an underlying
     * @ref offbynull::aligner::sequence::sequence::sequence, returning its elements as sliding window chunks of size `window_length`.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ Type of underlying @ref offbynull::aligner::sequence::sequence::sequence.
     * @tparam CONTAINER_CREATOR_PACK Type of container creator pack.
     */
    template<
        bool debug_mode,
        sequence SEQ,
        sliding_window_sequence_container_creator_pack<
            std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>
        > CONTAINER_CREATOR_PACK = sliding_window_sequence_heap_container_creator_pack<
            debug_mode,
            std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>
        >
    >
    class sliding_window_sequence {
    private:
        const SEQ& seq;
        const std::size_t window_length;

        using RESULT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_result_container(0zu));

        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence instance.
         *
         * @param seq_ Underlying sequence.
         * @param window_length_ Number of elements per sliding window chunk.
         * @param container_creator_pack_ Container creator pack.
         */
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
                    throw std::runtime_error { "Window length too large" };
                }
            }
        }

        /**
         * @copydoc offbynull::aligner::sequence::sequence::unimplemented_sequence::operator[]
         */
        RESULT_CONTAINER operator[](std::size_t index) const {
            RESULT_CONTAINER ret { container_creator_pack.create_result_container(window_length) };
            for (std::size_t i { 0zu }; i < window_length; i++) {
                ret[i] = seq[index + i];
            }
            return ret;
        }

        /**
         * @copydoc offbynull::aligner::sequence::sequence::unimplemented_sequence::size()
         */
        std::size_t size() const {
            return seq.size() - window_length + 1zu;
        }
    };


    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence instance using
     * an @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence_heap_container_creator_pack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seq Underlying sequence.
     * @param chunk_length Number of elements per chunk.
     * @return Newly created @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence instance.
     */
    template<bool debug_mode>
    auto create_heap_sliding_window_sequence(
        const sequence auto& seq,
        std::size_t chunk_length
    ) -> sliding_window_sequence<
        debug_mode,
        std::remove_cvref_t<decltype(seq)>
    > {
        return
            sliding_window_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(seq)>
            > { seq, chunk_length };
    }

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence instance using
     * an @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence_stack_container_creator_pack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam chunk_length Number of elements per chunk.
     * @param seq Underlying sequence.
     * @return Newly created @ref offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence instance.
     */
    template<bool debug_mode, std::size_t chunk_length>
    auto create_stack_sliding_window_sequence(
        const sequence auto& seq
    ) -> sliding_window_sequence<
        debug_mode,
        std::remove_cvref_t<decltype(seq)>,
        sliding_window_sequence_stack_container_creator_pack<
            debug_mode,
            std::remove_cvref_t<decltype(seq[0zu])>,
            chunk_length
        >
    > {
        using ELEM = std::remove_cvref_t<decltype(seq[0zu])>;
        return
             sliding_window_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(seq)>,
                sliding_window_sequence_stack_container_creator_pack<debug_mode, ELEM, chunk_length>
            > { seq, chunk_length };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H
