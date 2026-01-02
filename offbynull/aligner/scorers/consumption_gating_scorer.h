#ifndef OFFBYNULL_ALIGNER_SCORERS_CONSUMPTION_GATING_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_CONSUMPTION_GATING_SCORER_H

#include <utility>
#include <cstddef>
#include <optional>
#include <functional>
#include <stdexcept>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorers::consumption_gating_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight_or_seq_index;
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Maximum number of elements that can be consumed from a pair of sequences.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ_INDEX Sequence indexer type.
     */
    template<
        bool debug_mode,
        widenable_to_size_t SEQ_INDEX
    >
    struct consumption_gating_limits {
        /** Maximum elements consumable from the down sequence. */
        SEQ_INDEX down_consumed_max;
        /** Maximum elements consumable from the right sequence. */
        SEQ_INDEX right_consumed_max;

        friend bool operator==(
            const consumption_gating_limits<debug_mode, SEQ_INDEX>&,
            const consumption_gating_limits<debug_mode, SEQ_INDEX>&
        ) = default;
    };

    /**
     * Weight that wraps some other weight, causing it to always report less-than if too many sequence elements are consumed. That is, this
     * weight tracks how many elements have been consumed within both the down and right sequences, and if the consumption exceeds some
     * limit the weight always reports as being less-than.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ_INDEX Sequence indexer type.
     * @tparam BACKING_WEIGHT Backing weight type.
     */
    template<
        bool debug_mode,
        widenable_to_size_t SEQ_INDEX,
        weight BACKING_WEIGHT
    >
    struct consumption_gating_weight {
        /** Backing weight. */
        BACKING_WEIGHT real_weight;
        /** Number of down sequence elements consumed. */
        SEQ_INDEX down_consumed;
        /** Number of right sequence elements consumed. */
        SEQ_INDEX right_consumed;
        /** Maximum number of down sequence and right sequence elements that can be consumed. */
        std::reference_wrapper<
            const consumption_gating_limits<debug_mode, SEQ_INDEX>
        > limits;

        // TODO: Very real risk of limits being dangling reference if owning scorer is destroyed. Not sure what to do about this other than
        //       maintain an internal copy, which may end up consuming more memory than is needed.

        // TODO: Instead of setting std::optional<> for limits, maybe change the scorer interface such that you can ask it for an initial
        //       zero score. Or maybe just take an initial weight into the backtracker? That way, you can always have limits available / you
        //       can get rid of the std::optional overhead.

        friend consumption_gating_weight operator+(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            if constexpr (debug_mode) {
                if (a.limits.get() != b.limits.get()) {
                    throw std::runtime_error { "Mismatching limits" };
                }
            }
            return {
                static_cast<BACKING_WEIGHT>(a.real_weight + b.real_weight),
                static_cast<SEQ_INDEX>(a.down_consumed + b.down_consumed),
                static_cast<SEQ_INDEX>(a.right_consumed + b.right_consumed),
                a.limits
            };
        }

        friend consumption_gating_weight operator-(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            if constexpr (debug_mode) {
                if (a.limits.get() != b.limits.get()) {
                    throw std::runtime_error { "Mismatching limits" };
                }
            }
            return {
                static_cast<BACKING_WEIGHT>(a.real_weight - b.real_weight),
                static_cast<SEQ_INDEX>(a.down_consumed - b.down_consumed),
                static_cast<SEQ_INDEX>(a.right_consumed - b.right_consumed),
                a.limits
            };
        }

        friend bool operator<(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            if constexpr (debug_mode) {
                if (a.limits.get() != b.limits.get()) {
                    throw std::runtime_error { "Mismatching limits" };
                }
            }
            const auto& limits_ { a.limits.get() };
            bool a_exceeded { a.down_consumed > limits_.down_consumed_max || a.right_consumed > limits_.right_consumed_max };
            bool b_exceeded { b.down_consumed > limits_.down_consumed_max || b.right_consumed > limits_.right_consumed_max };
            if (a_exceeded && b_exceeded) {
                if constexpr (debug_mode) {
                    throw std::runtime_error { "Both exceeded - not comparable" };
                }
                std::unreachable();
            } else if (a_exceeded && !b_exceeded) {
                return true;  // a exceeded, so treat as if a less than b (treat it as smaller score so bigger score moves through)
            } else if (!a_exceeded && b_exceeded) {
                return false;  // b exceeded, so treat as if a not less than b (treat it as bigger score so smaller score moves through)
            } else {
                return a.real_weight < b.real_weight;
            }
        }

        friend bool operator==(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            return !(a < b) && !(b < a);
        }

        friend bool operator<=(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            return (a < b) || (a == b);
        }

        friend bool operator>(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            return !(a <= b);
        }

        friend bool operator>=(const consumption_gating_weight& a, const consumption_gating_weight& b) {
            return (a > b) || (a == b);
        }
    };

    static_assert(
        weight<
            consumption_gating_weight<
                true,
                std::size_t,
                std::size_t
            >
        >
    );

    /**
     * @ref offbynull::aligner::scorer::scorer::scorer that keeps track of how many elements have been consumed within each sequence, always
     * reporting lower once the number of elements exceeds some maximum.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     */
    template<
        bool debug_mode,
        typename DOWN_ELEM,
        typename RIGHT_ELEM,
        scorer_without_explicit_weight_or_seq_index<DOWN_ELEM, RIGHT_ELEM> BACKING_SCORER
    >
    class consumption_gating_scorer {
    public:
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::WEIGHT */
        using WEIGHT = consumption_gating_weight<
            debug_mode,
            typename BACKING_SCORER::SEQ_INDEX,
            typename BACKING_SCORER::WEIGHT
        >;

        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::SEQ_INDEX */
        using SEQ_INDEX = typename BACKING_SCORER::SEQ_INDEX;

    private:
        const consumption_gating_limits<debug_mode, SEQ_INDEX> consumption_limits;
        const BACKING_SCORER& backing_scorer;

    public:
        /**
         * Construct an @ref offbynull::aligner::scorers::consumption_gating_scorer::consumption_gating_scorer instance.
         *
         * @param max_down_consumption Maximum number of down elements allowed to be consumed.
         * @param max_right_consumption Maximum number of right elements allowed to be consumed.
         * @param backing_scorer_ Backing scorer.
         */
        consumption_gating_scorer(
            SEQ_INDEX max_down_consumption,
            SEQ_INDEX max_right_consumption,
            const BACKING_SCORER& backing_scorer_
        )
        : consumption_limits { max_down_consumption, max_right_consumption }
        , backing_scorer { backing_scorer_ } {}


        consumption_gating_scorer(SEQ_INDEX, SEQ_INDEX, BACKING_SCORER&&)=delete;
        consumption_gating_scorer(SEQ_INDEX, SEQ_INDEX, const BACKING_SCORER&&)=delete;

        // This must be operator()() - if you do operator(), doxygen won't recognize it. This isn't the case with other functions (if you
        // leave out the parenthesis, doxygen copies the documentation just fine).
        /**
         * @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::operator()()
         */
        WEIGHT operator()(
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const DOWN_ELEM>
                >
            > down_elem,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const RIGHT_ELEM>
                >
            > right_elem
        ) const {
            return {
                backing_scorer(down_elem, right_elem),
                static_cast<SEQ_INDEX>(down_elem.has_value() ? 1zu : 0zu),
                static_cast<SEQ_INDEX>(right_elem.has_value() ? 1zu : 0zu),
                consumption_limits
            };
        }
    };

    static_assert(
        scorer<
            consumption_gating_scorer<
                true,
                char,
                char,
                constant_scorer<
                    true,
                    std::size_t,
                    char,
                    char,
                    float
                >
            >,
            std::size_t,
            char,
            char,
            consumption_gating_weight<true, std::size_t, float>
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_CONSUMPTION_GATING_SCORER_H
