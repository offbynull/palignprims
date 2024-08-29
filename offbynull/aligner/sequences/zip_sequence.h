#ifndef OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <functional>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace offbynull::aligner::sequences::zip_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;

    template<typename... Args>
    auto make_ref_tuple(Args&... args) {
        return std::make_tuple(std::ref(args)...);
    }

    template<sequence T>
    static std::size_t min_size(std::size_t current_min, const T& first) {
        std::size_t next_size { first.size() };
        if (next_size < current_min) {
            current_min = next_size;
        }
        return current_min;
    }

    template<sequence T, sequence... OTHERS>
    static std::size_t min_size(std::size_t current_min, const T& first, const OTHERS& ... others) {
        std::size_t next_size { first.size() };
        if (next_size < current_min) {
            current_min = next_size;
        }
        return min_size(current_min, others...);
    }

    template<sequence T>
    auto access(std::size_t index, const T& seq) {
        return std::make_tuple(seq[index]);
    }

    template<sequence T, sequence... OTHERS>
    auto access(std::size_t index, const T& seq, const OTHERS& ... others) {
        return std::tuple_cat(
            access(index, seq),
            access(index, others...)
        );
    }

    template<bool debug_mode, sequence... SEQUENCES>
    class zip_sequence {
    private:
        const std::tuple<const SEQUENCES&...> seqs;
        const std::size_t size_;

    public:
        zip_sequence(const SEQUENCES&... seqs_)
        : seqs { std::tie(seqs_...) }
        , size_ { min_size(std::numeric_limits<std::size_t>::max(), seqs_...) } {}

        auto operator[](std::size_t index) const {
            auto params {
                std::tuple_cat(
                    std::make_tuple(index),
                    seqs
                )
            };
            //offbynull::utils::type_displayer<decltype(params)> x {};
            auto ret { std::apply(access<SEQUENCES...>, params) };
            //offbynull::utils::type_displayer<decltype(ret)> y {};
            return ret;
        }

        std::size_t size() const {
            return size_;
        }
    };





    template<typename... T>
    struct cvref_remover;

    template<typename T>
    struct cvref_remover<T> {
        using type = std::tuple<std::remove_cvref_t<T>>;
    };

    template<typename T, typename... OTHERS>
    struct cvref_remover<T, OTHERS...> {
        using type = decltype(
            std::tuple_cat(
                std::declval<std::tuple<std::remove_cvref_t<T>>>(),
                std::declval<typename cvref_remover<OTHERS...>::type>()
            )
        );
    };

    template<bool debug_mode, typename... TYPES>
    struct zip_sequencer_typer;

    template<bool debug_mode, typename... TYPES>
    struct zip_sequencer_typer<debug_mode, std::tuple<TYPES ...>> {
        using type = zip_sequence<debug_mode, TYPES ...>;
    };

    template<bool debug_mode>
    auto create_zip_sequence(const sequence auto&... seqs) {
        using TUPLE_PACKED_SEQ_TYPES = typename cvref_remover<decltype(seqs)...>::type;
        using ZIP_SEQUENCE_TYPE = typename zip_sequencer_typer<debug_mode, TUPLE_PACKED_SEQ_TYPES>::type;

        return ZIP_SEQUENCE_TYPE { seqs... };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H
