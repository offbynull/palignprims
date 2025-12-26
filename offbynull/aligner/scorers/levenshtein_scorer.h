#ifndef OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H

#include <cstddef>
#include <type_traits>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorers::levenshtein_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Levenshtein (string distance) @ref offbynull::aligner::scorer::scorer::scorer targeting various thresholds.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ_INDEX Sequence indexer type.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     * @tparam WEIGHT Pairwise alignment graph's edge data type (edge's weight).
     */
    template<bool debug_mode, widenable_to_size_t SEQ_INDEX, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    requires std::is_signed_v<WEIGHT>  // Must be signed because -1 used
    class levenshtein_scorer : public simple_scorer<debug_mode, SEQ_INDEX, DOWN_ELEM, RIGHT_ELEM, WEIGHT> {
    public:
        levenshtein_scorer()
        : simple_scorer<debug_mode, SEQ_INDEX, DOWN_ELEM, RIGHT_ELEM, WEIGHT> {
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(-1),
            static_cast<WEIGHT>(-1),
            static_cast<WEIGHT>(-1),
            static_cast<WEIGHT>(-1)
        } {}
    };

    static_assert(
        scorer<
            simple_scorer<true, std::size_t, char, char, float>,
            std::size_t,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H
