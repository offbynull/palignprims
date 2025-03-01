#ifndef OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * Zip @ref offbynull::aligner::sequence::sequence::sequence.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::aligner::sequences::zip_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;

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

    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence that combines many underlying
     * @ref offbynull::aligner::sequence::sequence::sequence objects together by zipping their elements (similar to Python's `zip()`).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQS List of underlying @ref offbynull::aligner::sequence::sequence::sequence types being zipped.
     */
    template<bool debug_mode, sequence... SEQS>
    class zip_sequence {
    private:
        const std::tuple<const SEQS&...> seqs;
        const std::size_t size_;

    public:
        /**
         * Construct an @ref offbynull::aligner::sequences::zip_sequence::zip_sequence instance.
         *
         * @param seqs_ Underlying sequences.
         */
        zip_sequence(const SEQS&... seqs_)
        : seqs { std::tie(seqs_...) }
        , size_ { min_size(std::numeric_limits<std::size_t>::max(), seqs_...) } {}

        /**
         * Get element at index `index`. Each element is a tuple containing the elements within the underlying sequences at `index`.
         *
         * @param index Index of element.
         * @return Element at `index`.
         */
        auto operator[](std::size_t index) const {
            auto params {
                std::tuple_cat(
                    std::make_tuple(index),
                    seqs
                )
            };
            //offbynull::utils::type_displayer<decltype(params)> x {};
            auto ret { std::apply(access<SEQS...>, params) };
            //offbynull::utils::type_displayer<decltype(ret)> y {};
            return ret;
        }

        /**
         * Get number of elements, which is the minimum `size()` across all underlying sequences.
         *
         * @return Number of elements.
         */
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

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::zip_sequence::zip_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param seqs Underlying sequences.
     * @return Newly created @ref offbynull::aligner::sequences::zip_sequence::zip_sequence instance.
     */
    template<bool debug_mode>
    auto create_zip_sequence(const sequence auto&... seqs) {
        using TUPLE_PACKED_SEQ_TYPES = typename cvref_remover<decltype(seqs)...>::type;
        using ZIP_SEQUENCE_TYPE = typename zip_sequencer_typer<debug_mode, TUPLE_PACKED_SEQ_TYPES>::type;

        return ZIP_SEQUENCE_TYPE { seqs... };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H
