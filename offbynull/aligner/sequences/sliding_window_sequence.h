#ifndef OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_SLIDING_WINDOW_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/helpers/container_creators.h"
#include <cstddef>
#include <array>
#include <type_traits>
#include <stdexcept>

namespace offbynull::aligner::sequences::sliding_window_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::small_vector_container_creator;

    template<bool error_check, sequence SEQ, std::size_t WINDOW_LENGTH>
    class compiletime_sliding_window_sequence {
    private:
        const SEQ& seq;

        using INNER_ELEM = std::decay_t<decltype(seq[0zu])>;

    public:
        compiletime_sliding_window_sequence(const SEQ& seq_)
        : seq { seq_ } {
            if constexpr (error_check) {
                if (seq.size() < WINDOW_LENGTH) {
                    throw std::runtime_error("Window length too large");
                }
            }
        }

        std::array<INNER_ELEM, WINDOW_LENGTH> operator[](std::size_t index) const {
            std::array<INNER_ELEM, WINDOW_LENGTH> ret { };
            for (std::size_t i { 0zu }; i < WINDOW_LENGTH; i++) {
                ret[i] = seq[index + i];
            }
            return ret;
        }

        std::size_t size() const {
            return seq.size() - WINDOW_LENGTH + 1zu;
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
    class runtime_sliding_window_sequence {
    private:
        const SEQ& seq;
        const std::size_t window_length;
        const CONTAINER_CREATOR container_creator;


        using INNER_ELEM = std::decay_t<decltype(seq[0zu])>;

    public:
        runtime_sliding_window_sequence(
            const SEQ& seq_,
            std::size_t window_length_,
            CONTAINER_CREATOR container_creator_ = {}
        )
        : seq { seq_ }
        , window_length { window_length_ }
        , container_creator { container_creator_ } {
            if constexpr (error_check) {
                if (seq.size() < window_length) {
                    throw std::runtime_error("Window length too large");
                }
            }
        }

        auto operator[](std::size_t index) const {
            auto ret { container_creator.create_objects({ window_length }) };
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
