#ifndef OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H

#include <utility>
#include <cstddef>
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
     * @tparam DOWN_ELEM Alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Alignment graph's rightward sequence element type.
     * @tparam WEIGHT Alignment graph's edge weight type.
     */
    template<bool debug_mode, widenable_to_size_t SEQ_INDEX, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    class levenshtein_scorer : public simple_scorer<debug_mode, SEQ_INDEX, DOWN_ELEM, RIGHT_ELEM, WEIGHT> {
    public:
        levenshtein_scorer()
        : simple_scorer<debug_mode, SEQ_INDEX, DOWN_ELEM, RIGHT_ELEM, WEIGHT> {
            static_cast<WEIGHT>(-1),
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(0)
        } {}
    };

    static_assert(
        scorer<
            simple_scorer<true, std::size_t, char, char, float>,
            std::pair<int, int>,
            std::size_t,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H
