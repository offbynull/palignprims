#ifndef OFFBYNULL_ALIGNER_ALIGNERS_INTERNAL_UTILS_H
#define OFFBYNULL_ALIGNER_ALIGNERS_INTERNAL_UTILS_H

#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/aligners/concepts.h"
#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <limits>
#include <boost/safe_numerics/checked_default.hpp>

namespace offbynull::aligner::aligners::internal_utils {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::numeric;
    using offbynull::utils::wider_numeric;
    using offbynull::aligner::aligners::concepts::alignment_result;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph;

    template<typename... Ts>
    struct widest_scorer_weight;

    template<typename s1>
    struct widest_scorer_weight<s1> {
        using type = typename s1::WEIGHT;
    };

    template<typename s1, typename ...rest>
    struct widest_scorer_weight<s1, rest...> {
        using type = typename wider_numeric<typename s1::WEIGHT, typename widest_scorer_weight<rest...>::type>::type;
    };

    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX,
        typename ...scorers_
    >
    auto validate_align_and_extract_widest_weight(
        const sequence auto& down,
        const sequence auto& right
    ) {
        // Check for rollover on +1
        if constexpr (debug_mode) {
            auto res1 { boost::safe_numerics::checked::add<N_INDEX>(down.size(), static_cast<N_INDEX>(1zu)) };
            auto res2 { boost::safe_numerics::checked::add<N_INDEX>(right.size(), static_cast<N_INDEX>(1zu)) };
            if (res1.exception() || res2.exception()) {
                throw std::runtime_error { "Index type too narrow" };
            }
        }
        // Check N_INDEX to ensure it can hold sequence
        if constexpr (debug_mode) {
            // Ensure sizes both are std::size_t. Why? because seq.size() only requires the return be widenably to std::size_t (as opposed
            // to actually being size_t), and so std::max won't work if the types being input are different (requies both  inputs to be of
            // the exact same type).
            std::size_t down_size { down.size() };
            std::size_t right_size { right.size() };
            // Will +1 cause the sequence to extend past std::size_t's max?
            // TODO: Remove this? No reasonably this can happen.
            if (std::max(down_size, right_size) == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error { "Sequence too large" };
            }
            // Sequence's size fits into scorers's SEQ_INDEX type?
            // TODO: +1zu not required here? This is SEQ_INDEX, not N_INDEX. But, if you don't do this, you'll potentially get narrowing
            //       warnings?
            std::size_t index_max { std::max(down_size, right_size) + 1zu };
            if (((index_max > std::numeric_limits<typename std::remove_cvref_t<scorers_>::SEQ_INDEX>::max()) || ...)) {
                throw std::runtime_error { "Scorer's sequence index type too narrow to support a sequence this large" };
            }
        }

        // NOTE: Technically, weight can be non-numeric (it just needs to have certain type traits), but in this case it must be
        //       numeric to determine which one is wider.
        static_assert((numeric<typename std::remove_cvref_t<scorers_>::WEIGHT> && ...), "weight must be numeric");
        return widest_scorer_weight<scorers_...> {};
    }
}

#endif //OFFBYNULL_ALIGNER_ALIGNERS_INTERNAL_UTILS_H