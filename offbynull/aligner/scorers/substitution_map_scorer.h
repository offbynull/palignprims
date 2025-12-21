#ifndef OFFBYNULL_ALIGNER_SCORERS_SUBSTITUTION_MAP_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_SUBSTITUTION_MAP_SCORER_H

#include <concepts>
#include <optional>
#include <map>
#include <utility>
#include <functional>
#include <cstddef>
#include <tuple>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorers::substitution_map_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * @ref offbynull::aligner::scorer::scorer::scorer which scores elements using a `std::map`.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ_INDEX Sequence indexer type.
     * @tparam DOWN_ELEM Alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Alignment graph's rightward sequence element type.
     * @tparam WEIGHT Alignment graph's edge weight type.
     */
    template<bool debug_mode, widenable_to_size_t SEQ_INDEX, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    requires requires (const DOWN_ELEM down_elem, const RIGHT_ELEM right_elem) {
        { down_elem < down_elem } -> std::same_as<bool>;
        { right_elem < right_elem } -> std::same_as<bool>;
    }
    class substitution_map_scorer {
    private:
        const std::map<std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>>, WEIGHT> data;

    public:
        /**
         * Construct an @ref offbynull::aligner::scorers::substitution_map_scorer::substitution_map_scorer instance.
         *
         * @param data_ Map of scores between element pairs (copied).
         */
        substitution_map_scorer(
            const std::map<std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>>, WEIGHT>& data_
        )
        : data { data_ } {}

        /**
         * Construct an @ref offbynull::aligner::scorers::substitution_map_scorer::substitution_map_scorer instance.
         *
         * @param data_ Map of scores between element pairs (moved).
         */
        substitution_map_scorer(
            std::map<std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>>, WEIGHT>&& data_
        )
        : data { std::move(data_) } {}

        // This must be operator()() - if you do operator(), doxygen won't recognize it. This isn't the case with other functions (if you
        // leave out the parenthesis, doxygen copies the documentation just fine).
        /**
         * @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::operator()()
         */
        WEIGHT operator()(
            [[maybe_unused]] const auto& edge,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const char>
                >
            > down_elem,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const char>
                >
            > right_elem
        ) const {
            if (down_elem.has_value() && right_elem.has_value()) {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { std::get<1>(*down_elem).get() },
                    { std::get<1>(*right_elem).get() }
                };
                return data.at(lookup);
            } else if (down_elem.has_value()) {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { std::get<1>(*down_elem).get() },
                    { std::nullopt }
                };
                return data.at(lookup);
            } else if (right_elem.has_value()) {
                std::pair<std::optional<DOWN_ELEM>, std::optional<RIGHT_ELEM>> lookup {
                    { std::nullopt },
                    { std::get<1>(*right_elem).get() }
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
            substitution_map_scorer<true, std::size_t, char, char, float>,
            std::pair<int, int>,
            std::size_t,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_SUBSTITUTION_MAP_SCORER_H
