#ifndef OFFBYNULL_ALIGNER_ALIGNERS_GLOBAL_DYNAMIC_PROGRAMMING_ALIGNMENT_TYPE_PARAMETERIZER_H
#define OFFBYNULL_ALIGNER_ALIGNERS_GLOBAL_DYNAMIC_PROGRAMMING_ALIGNMENT_TYPE_PARAMETERIZER_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::aligners::global_dynamic_programming_alignment_type_parameterizer {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::narrowest_type_for_indexing;

    /**
     * Parameters for dynamic programming global alignment.
     *
     * @tparam N_INDEX Pairwise alignment graph grid coordinate type. For example, `std::uint8_t` will allow up to 255 node per axim,
     *     meaning it'll support sequences of lengths up to 255-1.
     */
    template<widenable_to_size_t N_INDEX>
    struct global_dynamic_programming_alignment_type_parameterizer {
    public:
        /**
         * See @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker's PARENT_COUNT type
         * parameter. For global alignment, the type must be able to hold a maximum value of 3.
         */
        using PARENT_COUNT = std::uint8_t;  // This will never be more than 3 for global alignment graph

    private:
        static constexpr std::size_t max_index { std::numeric_limits<N_INDEX>::max() };
        static constexpr std::size_t max_slots { max_index * max_index * 1zu };  // down * right * depth

    public:
        /**
         * See @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker's SLOT_INDEX type
         * parameter.
         */
        using SLOT_INDEX = typename narrowest_type_for_indexing<max_slots - 1uz>::type;  // max_slots-1 because we want max idx, not size
    };
}

#endif //OFFBYNULL_ALIGNER_ALIGNERS_GLOBAL_DYNAMIC_PROGRAMMING_ALIGNMENT_TYPE_PARAMETERIZER_H