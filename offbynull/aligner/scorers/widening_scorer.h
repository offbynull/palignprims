#ifndef OFFBYNULL_ALIGNER_SCORERS_WIDENING_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_WIDENING_SCORER_H

#include <utility>
#include <cstddef>
#include <optional>
#include <functional>
#include <type_traits>
#include <cstdint>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::scorers::widening_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight_or_seq_index;
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::narrower_numeric;
    using offbynull::utils::wider_numeric;

    /**
     * @ref offbynull::aligner::scorer::scorer::scorer which forwards to a backing scorer, ensuring values being passed back and forth are
     * widenable (narrowing not allowed).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ_INDEX_ Sequence indexer type.
     * @tparam DOWN_ELEM Pairwise alignment graph's downward sequence element type.
     * @tparam RIGHT_ELEM Pairwise alignment graph's rightward sequence element type.
     * @tparam WEIGHT_ Pairwise alignment graph's edge data type (edge's weight).
     * @tparam BACKING_SCORER Backing score type.
     */
    template<
        bool debug_mode,
        widenable_to_size_t SEQ_INDEX_,
        typename DOWN_ELEM,
        typename RIGHT_ELEM,
        weight WEIGHT_,
        scorer_without_explicit_weight_or_seq_index<DOWN_ELEM, RIGHT_ELEM> BACKING_SCORER
    >
    requires std::is_same_v<typename wider_numeric<WEIGHT_, typename BACKING_SCORER::WEIGHT>::type, WEIGHT_>
        && std::is_same_v<typename narrower_numeric<SEQ_INDEX_, typename BACKING_SCORER::SEQ_INDEX>::type, SEQ_INDEX_>
    class widening_scorer {
    public:
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::WEIGHT */
        using WEIGHT = WEIGHT_;
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::SEQ_INDEX */
        using SEQ_INDEX = SEQ_INDEX_;

    private:
        const BACKING_SCORER& backing_scorer;

        template<typename ELEM>
        std::optional<
            std::pair<
                SEQ_INDEX,
                std::reference_wrapper<const ELEM>
            >
        > cast_scorer_element(const auto& e) const {
            if (!e.has_value()) {
                return { std::nullopt };
            }
            const auto&[idx, elem] { *e };
            return std::optional {
                std::pair<SEQ_INDEX, std::remove_cvref_t<decltype(elem)>> {
                    static_cast<SEQ_INDEX>(idx),
                    elem
                }
            };
        }

    public:
        /**
         * Construct an @ref offbynull::aligner::scorers::widening_scorer::widening_scorer instance.
         *
         * @param backing_scorer_ Scorer to forward to.
         */
        widening_scorer(const BACKING_SCORER& backing_scorer_)
        : backing_scorer { backing_scorer_ } {}

        widening_scorer(BACKING_SCORER&&) = delete; // No rvalues
        widening_scorer(const BACKING_SCORER&&) = delete; // No rvalues, even if a const (how do you move a const rvalue - makes no sense?)

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
            using BACKING_SCORER_WEIGHT = typename BACKING_SCORER::WEIGHT;

            const auto& new_down_elem { cast_scorer_element<DOWN_ELEM>(down_elem) };
            const auto& new_right_elem { cast_scorer_element<RIGHT_ELEM>(right_elem) };
            BACKING_SCORER_WEIGHT ret { backing_scorer(new_down_elem, new_right_elem) };
            return static_cast<WEIGHT>(ret);
        }
    };

    static_assert(
        scorer<
            widening_scorer<
                true,
                std::uint8_t,
                char,
                char,
                double,
                constant_scorer<
                    true,
                    std::size_t,
                    char,
                    char,
                    float
                >
            >,
            std::uint8_t,
            char,
            char,
            double
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_WIDENING_SCORER_H
