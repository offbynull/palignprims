#ifndef OFFBYNULL_ALIGNER_SCORERS_SIMPLE_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_SIMPLE_SCORER_H

#include <concepts>
#include <optional>
#include <limits>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::scorers::simple_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;

    template<bool error_check, typename DOWN_ELEM, typename RIGHT_ELEM, weight WEIGHT>
    requires requires (const DOWN_ELEM down_elem, const RIGHT_ELEM right_elem) {
        { down_elem == right_elem } -> std::same_as<bool>;
    }
    class simple_scorer {
    private:
        const WEIGHT match_weight;
        const WEIGHT mismatch_weight;
        const WEIGHT down_missing_weight;
        const WEIGHT right_missing_weight;
        const WEIGHT both_missing_weight;

    public:
        simple_scorer(
            WEIGHT match_weight_,
            WEIGHT mismatch_weight_,
            WEIGHT down_missing_weight_,
            WEIGHT right_missing_weight_,
            WEIGHT both_missing_weight_
        )
        : match_weight { match_weight_ }
        , mismatch_weight { mismatch_weight_ }
        , down_missing_weight { down_missing_weight_ }
        , right_missing_weight { right_missing_weight_ }
        , both_missing_weight { both_missing_weight_ } {}

        static simple_scorer<error_check, DOWN_ELEM, RIGHT_ELEM, WEIGHT> create_substitution(
            WEIGHT match_weight_,
            WEIGHT mismatch_weight_
        ) {
            constexpr bool nan_available { std::numeric_limits<WEIGHT>::has_quiet_NaN };
            constexpr WEIGHT missing_val { nan_available ? std::numeric_limits<WEIGHT>::quiet_NaN() : std::numeric_limits<WEIGHT>::max() };
            return {
                match_weight_,
                mismatch_weight_,
                missing_val,
                missing_val,
                missing_val
            };
        }

        static simple_scorer<error_check, DOWN_ELEM, RIGHT_ELEM, WEIGHT> create_gap(
            WEIGHT gap_weight_
        ) {
            constexpr bool nan_available { std::numeric_limits<WEIGHT>::has_quiet_NaN };
            constexpr WEIGHT missing_val { nan_available ? std::numeric_limits<WEIGHT>::quiet_NaN() : std::numeric_limits<WEIGHT>::max() };
            return {
                missing_val,
                missing_val,
                gap_weight_,
                gap_weight_,
                missing_val
            };
        }

        static simple_scorer<error_check, DOWN_ELEM, RIGHT_ELEM, WEIGHT> create_gap_asymmetric(
            WEIGHT down_gap_weight_,
            WEIGHT right_gap_weight_
        ) {
            constexpr bool nan_available { std::numeric_limits<WEIGHT>::has_quiet_NaN };
            constexpr WEIGHT missing_val { nan_available ? std::numeric_limits<WEIGHT>::quiet_NaN() : std::numeric_limits<WEIGHT>::max() };
            return {
                missing_val,
                missing_val,
                down_gap_weight_,
                right_gap_weight_,
                missing_val
            };
        }

        static simple_scorer<error_check, DOWN_ELEM, RIGHT_ELEM, WEIGHT> create_freeride(
            WEIGHT freeride_weight_ = {}
        ) {
            constexpr bool nan_available { std::numeric_limits<WEIGHT>::has_quiet_NaN };
            constexpr WEIGHT missing_val { nan_available ? std::numeric_limits<WEIGHT>::quiet_NaN() : std::numeric_limits<WEIGHT>::max() };
            return {
                missing_val,
                missing_val,
                missing_val,
                missing_val,
                freeride_weight_
            };
        }

        WEIGHT operator()(
            const auto& edge,
            const std::optional<std::reference_wrapper<const DOWN_ELEM>> down_elem,
            const std::optional<std::reference_wrapper<const RIGHT_ELEM>> right_elem
        ) const {
            if (down_elem.has_value() && right_elem.has_value()) {
                if (down_elem.value().get() == right_elem.value().get()) {
                    return match_weight;
                } else {
                    return mismatch_weight;
                }
            } else if (!down_elem.has_value() && right_elem.has_value()) {
                return down_missing_weight;
            } else if (down_elem.has_value() && !right_elem.has_value()) {
                return right_missing_weight;
            } else {
                return both_missing_weight;
            }
        }
    };

    static_assert(
        scorer<
            simple_scorer<true, char, char, float>,
            std::pair<int, int>,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_SIMPLE_SCORER_H
