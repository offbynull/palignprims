#ifndef OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <type_traits>
#include <utility>

/**
 * @ref offbynull::aligner::sequence::sequence::sequence that applies a transformation to each element.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::aligner::sequences::transform_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `T` has the traits of a transformer. A transformer is simply a callable object that transforms an object
     * from one type to another.
     *
     * @tparam T Type to check.
     * @tparam INPUT Type to transform from.
     */
    template<typename T, typename INPUT>
    concept transformer =
        // leave out unqualified_value_type<T> because it won't pass if T is a function pointer? or maybe it will?
        requires(const T t, INPUT input) {
            { t(input) } -> unqualified_object_type;
        };


    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that wraps an underlying
     * @ref offbynull::aligner::sequence::sequence::sequence and transforms its elements.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ Type of underlying @ref offbynull::aligner::sequence::sequence::sequence.
     * @tparam TRANSFORMER Type of transformer.
     */
    template<
        bool debug_mode,
        sequence SEQ,
        transformer<std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>> TRANSFORMER
    >
    class transform_sequence {
    private:
        const SEQ& seq;
        const TRANSFORMER transformer;

        using INPUT = std::remove_cvref_t<decltype(std::declval<SEQ>()[0zu])>;
        using OUTPUT = std::invoke_result_t<TRANSFORMER, INPUT>;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::transform_sequence::transform_sequence instance.
         *
         * @param seq_ Underlying sequence.
         * @param transformer_ Transformer.
         */
        transform_sequence(
            const SEQ& seq_,
            const TRANSFORMER& transformer_
        )
        : seq { seq_ }
        , transformer { transformer_ } {}

        /**
         * Get element at index `index`.
         *
         * @param index Index of element.
         * @return Value at `index`.
         */
        OUTPUT operator[](std::size_t index) const {
            INPUT input { seq[index] };
            return transformer(input);
        }

        /**
         * Get number of elements.
         *
         * @return Number of elements.
         */
        std::size_t size() const {
            return seq.size();
        }
    };


    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::transform_sequence::transform_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seq Underlying sequence.
     * @param transformer_ Transformer.
     * @return Newly created @ref offbynull::aligner::sequences::transform_sequence::transform_sequence instance.
     */
    template<bool debug_mode>
    auto create_transform_sequence(
        const sequence auto& seq,
        const transformer<std::remove_cvref_t<decltype(seq[0zu])>> auto& transformer_
    ) {
        return transform_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(seq)>,
                std::remove_cvref_t<decltype(transformer_)>
            > { seq, transformer_ };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H
