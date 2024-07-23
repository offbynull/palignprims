#ifndef OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H

#include <concepts>
#include <optional>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/scorers/simple_scorer.h"

namespace offbynull::aligner::scorers::levenshtein_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    template<typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    class levenshtein_scorer : public simple_scorer<DOWN_ELEM, RIGHT_ELEM, WEIGHT> {
    public:
        levenshtein_scorer()
        : simple_scorer<DOWN_ELEM, RIGHT_ELEM, WEIGHT> {
            static_cast<WEIGHT>(-1),
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(0),
            static_cast<WEIGHT>(0)
        } {}
    };

    static_assert(
        scorer<
            simple_scorer<char, char, float>,
            std::pair<int, int>,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_LEVENSHTEIN_SCORER_H
