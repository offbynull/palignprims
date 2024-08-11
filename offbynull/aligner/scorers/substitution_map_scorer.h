#ifndef OFFBYNULL_ALIGNER_SCORERS_SUBSTITUTION_MAP_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_SUBSTITUTION_MAP_SCORER_H

#include <concepts>
#include <optional>
#include <map>
#include <charconv>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::scorers::substitution_map_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;

    template<bool debug_mode, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    requires requires (const DOWN_ELEM down_elem, const RIGHT_ELEM right_elem) {
        { down_elem < down_elem } -> std::same_as<bool>;
        { right_elem < right_elem } -> std::same_as<bool>;
        { down_elem == right_elem } -> std::same_as<bool>;
    }
    class substitution_map_scorer {
    private:
        const std::map<std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>>, WEIGHT> data;

    public:
        substitution_map_scorer(
            const std::map<std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>>, WEIGHT>& data_
        )
        : data { data_ } {}

        substitution_map_scorer(
            std::map<std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>>, WEIGHT>&& data_
        )
        : data { std::move(data_) } {}

        WEIGHT operator()(
            const auto& edge,
            const std::optional<std::reference_wrapper<const DOWN_ELEM>> down_elem,
            const std::optional<std::reference_wrapper<const RIGHT_ELEM>> right_elem
        ) const {
            if (down_elem.has_value() && right_elem.has_value()) {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { down_elem.value().get() },
                    { right_elem.value().get() }
                };
                return data.at(lookup);
            } else if (down_elem.has_value()) {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { down_elem.value().get() },
                    { std::nullopt }
                };
                return data.at(lookup);
            } else if (right_elem.has_value()) {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { std::nullopt },
                    { right_elem.value().get() }
                };
                return data.at(lookup);
            } else {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { std::nullopt },
                    { std::nullopt }
                };
                return data.at(lookup);
            }
        }
    };

    static_assert(
        scorer<
            substitution_map_scorer<true, char, char, float>,
            std::pair<int, int>,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_SUBSTITUTION_MAP_SCORER_H
