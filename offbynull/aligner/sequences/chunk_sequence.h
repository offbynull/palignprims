#ifndef OFFBYNULL_ALIGNER_SEQUENCES_CHUNK_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_CHUNK_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <array>
#include <vector>
#include <ranges>
#include <type_traits>
#include <utility>
#include <stdexcept>

/**
 * Chunked @ref offbynull::aligner::sequence::sequence::sequence.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::aligner::sequences::chunk_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_object_type;


    /**
     * Concept that's satisfied if `T` has the traits for creating the containers required by
     * @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence, referred to as a container creator pack.
     *
     * @tparam T Type to check.
     * @tparam E Type of element within sequence.
     */
    template<
        typename T,
        typename E
    >
    concept chunk_sequence_container_creator_pack =
        unqualified_object_type<T>
        && requires(const T t, std::size_t chunk_len) {
            // TODO: Return concept check is wrong? Random access range doesn't allow write access via [] operator, only read access? Add
            //       check to see if std::ranges::range_reference_t<R> is a non-const reference?
            { t.create_chunk_container(chunk_len) } -> random_access_range_of_type<E>;
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
    struct chunk_sequence_heap_container_creator_pack {
        /**
         * Create chunk container.
         *
         * @param chunk_len Number of elements in chunk (must match `chunk_len`)
         * @return New container.
         */
        std::vector<E> create_chunk_container(std::size_t chunk_len) const {
            return std::vector<E>(chunk_len);
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
    struct chunk_sequence_stack_container_creator_pack {
        /**
         * Create chunk container.
         *
         * @param chunk_len_ Number of elements in chunk (must match `chunk_len`)
         * @return New container.
         * @throws std::runtime_error If `debug_mode && chunk_len != chunk_len_`.
         */
        std::array<E, chunk_len> create_chunk_container(std::size_t chunk_len_) const {
            if constexpr (debug_mode) {
                if (chunk_len != chunk_len_) {
                    throw std::runtime_error { "Bad element count" };
                }
            }
            return {};
        }
    };


    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that wraps an underlying
     * @ref offbynull::aligner::sequence::sequence::sequence, returning its elements as chunks of size `chunk_length`. If the number of
     * elements in the underlying @ref offbynull::aligner::sequence::sequence::sequence isn't cleanly divisible by `chunk_length`, *the
     * remainder is truncated*.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ Type of underlying @ref offbynull::aligner::sequence::sequence::sequence.
     * @tparam CONTAINER_CREATOR_PACK Type of container creator pack.
     */
    template<
        bool debug_mode,
        sequence SEQ,
        chunk_sequence_container_creator_pack<
            std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>
        > CONTAINER_CREATOR_PACK = chunk_sequence_heap_container_creator_pack<
            debug_mode,
            std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>
        >
    >
    class chunk_sequence {
    private:
        const SEQ& seq;
        const std::size_t chunk_length;

        using CHUNK_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_chunk_container(0zu));

        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence instance.
         *
         * @param seq_ Underlying sequence.
         * @param chunk_length_ Number of elements per chunk.
         * @param container_creator_pack_ Container creator pack.
         */
        chunk_sequence(
            const SEQ& seq_,
            std::size_t chunk_length_,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : seq { seq_ }
        , chunk_length { chunk_length_ }
        , container_creator_pack { container_creator_pack_ } {}

        /**
         * Get element (chunk) at index `index`.
         *
         * @param index Index of chunk.
         * @return Chunk at `index`.
         */
        CHUNK_CONTAINER operator[](std::size_t index) const {
            std::size_t offset { index * chunk_length };
            CHUNK_CONTAINER ret { container_creator_pack.create_chunk_container(chunk_length) };
            for (std::size_t i { 0zu }; i < chunk_length; i++) {
                ret[i] = seq[offset + i];
            }
            return ret;
        }

        /**
         * Get number of elements (chunks).
         *
         * @return Number of chunks.
         */
        std::size_t size() const {
            return seq.size() / chunk_length;
        }
    };

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence instance using an
     * @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence_heap_container_creator_pack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seq Underlying sequence.
     * @param chunk_length Number of elements per chunk.
     * @return Newly created @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence instance.
     */
    template<bool debug_mode>
    auto create_heap_chunk_sequence(const sequence auto& seq, std::size_t chunk_length) {
        return
            chunk_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(seq)>
            > { seq, chunk_length };
    }

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence instance using an
     * @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence_stack_container_creator_pack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam chunk_length Number of elements per chunk.
     * @param seq Underlying sequence.
     * @return Newly created @ref offbynull::aligner::sequences::chunk_sequence::chunk_sequence instance.
     */
    template<bool debug_mode, std::size_t chunk_length>
    auto create_stack_chunk_sequence(const sequence auto& seq) {
        using ELEM = std::remove_cvref_t<decltype(seq[0zu])>;
        return
             chunk_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(seq)>,
                chunk_sequence_stack_container_creator_pack<debug_mode, ELEM, chunk_length>
            > { seq, chunk_length };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_CHUNK_SEQUENCE_H
