#ifndef OFFBYNULL_UTILS_H
#define OFFBYNULL_UTILS_H

#include <cstddef>
#include <functional>

#include "utils.h"
#include "boost/container/static_vector.hpp"
#include "boost/container/options.hpp"

namespace offbynull::utils {
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
}

#endif //OFFBYNULL_UTILS_H
