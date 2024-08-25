#ifndef OFFBYNULL_UTILS_H
#define OFFBYNULL_UTILS_H

#include <cstddef>
#include <concepts>
#include <vector>
#include <ranges>
#include <type_traits>
#include <boost/container/static_vector.hpp>
#include <boost/container/options.hpp>

namespace offbynull::utils {
    /**
     * Unimplemented class template used as a hack to determine what some unknown type \c T is. Because this class template remains
     * unimplemented for every \c T, declararing it a variable of type \c type_displayer<T> will result in a compiler error that explicitly
     * states what \c T is.
     *
     * \code{.cpp}
     * void my_func(const auto& x) {
     *     type_displayer<decltype(x)> unused;  // error: 'type_displayer<const int&> unused' has incomplete type
     * }
     * \endcode
     *
     * @tparam T Type to display.
     */
    template<typename T>
    struct type_displayer;

    template<bool debug_mode, typename ELEM, std::size_t cnt>
    struct static_vector_typer;

    template<typename ELEM, std::size_t cnt>
    struct static_vector_typer<true, ELEM, cnt> {
        using type = boost::container::static_vector<ELEM, cnt>;
    };

    template<typename ELEM, std::size_t cnt>
    struct static_vector_typer<false, ELEM, cnt> {
        using type = boost::container::static_vector<
            ELEM,
            cnt,
            boost::container::static_vector_options<
                boost::container::throw_on_overflow<false>,
                boost::container::inplace_alignment<0u>
            >::type
        >;
    };

    template<typename T>
    concept integral_or_floating_point = std::integral<T> || std::floating_point<T>;

    template<integral_or_floating_point T>
    struct constants {
        constexpr static T _0 { static_cast<T>(0) };
        constexpr static T _1 { static_cast<T>(1) };
    };

    auto copy_to_vector(std::ranges::range auto&& range) -> std::vector<std::decay_t<decltype(*range.begin())>> {
        using ELEM = std::decay_t<decltype(*range.begin())>;
        std::vector<ELEM> ret {};
        for (const auto& e : range) {
            ret.push_back(e);
        }
        return ret;
    };
}

#endif //OFFBYNULL_UTILS_H
