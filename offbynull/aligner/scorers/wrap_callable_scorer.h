#ifndef OFFBYNULL_ALIGNER_SCORERS_WRAP_CALLABLE_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_WRAP_CALLABLE_SCORER_H

#include <utility>
#include <cstddef>
#include <optional>
#include <functional>
#include <type_traits>
#include <tuple>
#include <cstdint>
#include <boost/callable_traits/args.hpp>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::scorers::wrap_callable_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight_or_seq_index;
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::utils::narrower_numeric;
    using offbynull::utils::wider_numeric;

    // Utility functions used to extract types nested within backing scorer's callable type parameters.
    template<typename T> struct optional_inner;
    template<typename U> struct optional_inner<std::optional<U>> { using type = U; };

    template<typename T> struct reference_wrapper_inner;
    template<typename U> struct reference_wrapper_inner<std::reference_wrapper<const U>> { using type = U; };

    template<typename T> struct pair_first;
    template<typename A, typename B> struct pair_first<std::pair<A,B>> { using type = A; };

    template<typename T> struct pair_second;
    template<typename A, typename B> struct pair_second<std::pair<A,B>> { using type = B; };

    template<typename T>
    using seq_index_1_t = std::remove_cvref_t<
        typename pair_first<  // std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>
                              //          '-------------------'
            typename optional_inner<  // std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>
                                      //              '----------------------------------------------------------------------'
                std::tuple_element_t<0zu, boost::callable_traits::args_t<T>>  // 1st param
            >::type
        >::type
    >;
    template<typename T>
    using seq_index_2_t = std::remove_cvref_t<
        typename pair_first<  // std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>
                              //          '-------------------'
            typename optional_inner<  // std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>
                                      //              '----------------------------------------------------------------------'
                std::tuple_element_t<1zu, boost::callable_traits::args_t<T>>  // 2nd param
            >::type
        >::type
    >;

    template<typename T>
    using down_elem_t = typename reference_wrapper_inner<  // std::reference_wrapper<const DOWN_ELEM>
                                                           //                             '---------'
        typename pair_second<  // std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>
                               //                               '---------------------------------------'
            typename optional_inner<  // std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>
                                      //              '----------------------------------------------------------------------'
                std::tuple_element_t<0zu, boost::callable_traits::args_t<T>>  // 1st param
            >::type
        >::type
    >::type;
    template<typename T>
    using right_elem_t = typename reference_wrapper_inner<  // std::reference_wrapper<const RIGHT_ELEM>
                                                            //                             '----------'
        typename pair_second<  // std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>
                               //                               '----------------------------------------'
            typename optional_inner<  // std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>
                                      //              '-----------------------------------------------------------------------'
                std::tuple_element_t<1zu, boost::callable_traits::args_t<T>>  // 2nd param
            >::type
        >::type
    >::type;

    /**
     * Concept that's satisfied if `T` has the traits to be a
     * @ref offbynull::aligner::scorers::wrap_callable_scorer::wrap_callable_scorer's callable. `T` must be concrete type (no missing
     * template parameters) that's a callable with parameters similar to a @ref offbynull::aligner::scorer::scorer::scorer's invocation
     * parameters (not exactly the same, but similar):
     *
     *  1. const std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>>
     *  2. const std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>>
     *
     * See @ref offbynull::aligner::scorers::wrap_callable_scorer::wrap_callable_scorer's documentation for more information.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept wrap_callable_scorer_callable =
        unqualified_object_type<T>
        && widenable_to_size_t<seq_index_1_t<T>>
        && widenable_to_size_t<seq_index_2_t<T>>
        && requires { typename down_elem_t<T>; }  // No checking type, just checking to see that it resolves
        && requires { typename right_elem_t<T>; }  // No checking type, just checking to see that it resolves
        && requires (const T& t) {
            { t(std::nullopt, std::nullopt) } -> weight;
        };

    // WARNING: This is incredibly finicky to use, producing wild compiler errors if the requirements aren't met. It was okay as a thought
    //          experiment but there is more harm to using it vs just directly maing a scorer class where SEQ_INDEX and WEIGHT are directly
    //          encoded.

    /**
     * @ref offbynull::aligner::scorer::scorer::scorer which forwards to a callable, automatically deriving the required type aliases. The
     * callable passed in must be concrete type (no missing template parameters) that's a callable with the following parameters:
     *
     *  1. const std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>>
     *  2. const std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>>
     *
     * These parameters are similar to @ref offbynull::aligner::scorer::scorer::scorer's callable parameters, with the exception that
     * the sequence indexer type (SEQ_INDEX) doesn't have to be the same between the two parameters. The narrower one will be chosen as
     * this scorer's SEQ_INDEX.
     *
     * ```
     * const std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const DOWN_ELEM>>>
     *                              '---------+---------'
     *                                        |
     *                                        | these don't have to match, the narrower one will be chosen
     *                                        |
     *                              .---------+---------.
     * const std::optional<std::pair<widenable_to_size_t, std::reference_wrapper<const RIGHT_ELEM>>>
     * ```
     *
     * In the example below, the resulting scorer will have a SEQ_INDEX type of std::uint16_t (because std::uint16_t is narrower than
     * std::uint64_t) and a WEIGHT type of std::uint8_t (because that's what the lambda returns).
     *
     * ```
     * auto callable {
     *     [](
     *         const std::optional<
     *             std::pair<
     *                 std::uint64_t
     *                 std::reference_wrapper<const char>
     *             >
     *         > down_elem,
     *         const std::optional<
     *             std::pair<
     *                 std::uint16_t,
     *                 std::reference_wrapper<const int>
     *             >
     *         > right_elem
     *     ) {
     *         return static_cast<std::uint8_t>(d_elem - r_elem);
     *     }
     * };
     * wrap_callable_scorer<
     *     false,
     *     std::remove_cvref_t<decltype(callable)>
     * > scorer { callable };
     * ```
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam BACKING_CALLABLE Backing callable type.
     */
    template<
        bool debug_mode,
        wrap_callable_scorer_callable BACKING_CALLABLE
    >
    class wrap_callable_scorer {
    private:
        const BACKING_CALLABLE& backing_callable;

        using SEQ_INDEX_1 = seq_index_1_t<BACKING_CALLABLE>;
        using SEQ_INDEX_2 = seq_index_2_t<BACKING_CALLABLE>;

    public:
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::WEIGHT */
        using WEIGHT = decltype(backing_callable(std::nullopt, std::nullopt));
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::SEQ_INDEX */
        using SEQ_INDEX = typename narrower_numeric<SEQ_INDEX_1, SEQ_INDEX_2>::type;
        /** Downward element type. */
        using DOWN_ELEM = down_elem_t<BACKING_CALLABLE>;
        /** Rightward element type. */
        using RIGHT_ELEM = right_elem_t<BACKING_CALLABLE>;

    private:
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
         * Construct an @ref offbynull::aligner::scorers::wrap_callable_scorer::wrap_callable_scorer instance.
         *
         * @param backing_callable_ Callable to forward to (must be concrete - no missing template parameters).
         */
        wrap_callable_scorer(const BACKING_CALLABLE& backing_callable_)
        : backing_callable { backing_callable_ } {}

        wrap_callable_scorer(BACKING_CALLABLE&&) = delete; // No rvalues
        wrap_callable_scorer(const BACKING_CALLABLE&&) = delete; // No rvalues, even if a const (how do you move a const rvalue???)

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
            const auto& new_down_elem { cast_scorer_element<DOWN_ELEM>(down_elem) };
            const auto& new_right_elem { cast_scorer_element<RIGHT_ELEM>(right_elem) };
            return backing_callable(new_down_elem, new_right_elem);
        }
    };

    static_assert(
        scorer<
            wrap_callable_scorer<
                true,
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
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_WRAP_CALLABLE_SCORER_H
