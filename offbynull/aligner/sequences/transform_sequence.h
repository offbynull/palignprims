#ifndef OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <type_traits>
#include <utility>

namespace offbynull::aligner::sequences::transform_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_value_type;

    template<typename T, typename INPUT>
    concept transformer =
        // leave out unqualified_value_type<T> because it won't pass if T is a function pointer? or maybe it will?
        requires(const T t, INPUT input) {
            { t(input) } -> unqualified_value_type;
        };


    template<
        bool debug_mode,
        sequence BACKING_SEQUENCE,
        transformer<std::remove_cvref_t<decltype(std::declval<BACKING_SEQUENCE>()[0zu])>> TRANSFORMER
    >
    class transform_sequence {
    private:
        const BACKING_SEQUENCE& backing_sequence;
        const TRANSFORMER transformer;

        using INPUT = std::remove_cvref_t<decltype(std::declval<BACKING_SEQUENCE>()[0zu])>;
        using OUTPUT = std::invoke_result_t<TRANSFORMER, INPUT>;

    public:
        transform_sequence(
            const BACKING_SEQUENCE& backing_sequence_,
            const TRANSFORMER& transformer_
        )
        : backing_sequence { backing_sequence_ }
        , transformer { transformer_ } {}

        OUTPUT operator[](std::size_t index) const {
            INPUT input { backing_sequence[index] };
            return transformer(input);
        }

        std::size_t size() const {
            return backing_sequence.size();
        }
    };

    template<bool debug_mode>
    sequence auto create_transform_sequence(const sequence auto& backing_sequence_, const auto& transformer_) {
        return transform_sequence<
                debug_mode,
                std::remove_cvref_t<decltype(backing_sequence_)>,
                std::remove_cvref_t<decltype(transformer_)>
            > { backing_sequence_, transformer_ };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_TRANSFORM_SEQUENCE_H
