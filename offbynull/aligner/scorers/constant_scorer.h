#ifndef OFFBYNULL_ALIGNER_SCORERS_CONSTANT_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_CONSTANT_SCORER_H

#include <utility>
#include <optional>
#include <functional>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::scorers::constant_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;

    template<bool debug_mode, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    class constant_scorer {
    private:
        const WEIGHT weight;

    public:
        constant_scorer(WEIGHT weight_)
        : weight { weight_ } {}

        WEIGHT operator()(
            const auto& /*edge*/,
            const std::optional<std::reference_wrapper<const DOWN_ELEM>> /*down_elem*/,
            const std::optional<std::reference_wrapper<const RIGHT_ELEM>> /*right_elem*/
        ) const {
            return weight;
        }
    };

    static_assert(
        scorer<
            constant_scorer<true, char, char, float>,
            std::pair<int, int>,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_CONSTANT_SCORER_H
