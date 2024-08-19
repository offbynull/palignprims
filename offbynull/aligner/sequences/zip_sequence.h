#ifndef OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include <cstddef>
#include <utility>
#include <tuple>

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
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_ZIP_SEQUENCE_H
