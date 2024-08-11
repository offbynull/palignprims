#ifndef OFFBYNULL_ALIGNER_SCORERS_CONSTANT_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_CONSTANT_SCORER_H

#include <concepts>
#include <optional>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::scorers::constant_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;

    template<bool debug_mode, weight WEIGHT>
    class constant_scorer {
    private:
        const WEIGHT weight;

    public:
        constant_scorer(WEIGHT weight_)
        : weight { weight_ } {}

        WEIGHT operator()(
            const auto& edge,
            const auto& down_elem,
            const auto& right_elem
        ) const {
            return weight;
        }
    };

    static_assert(
        scorer<
            constant_scorer<true, float>,
            std::pair<int, int>,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_CONSTANT_SCORER_H
