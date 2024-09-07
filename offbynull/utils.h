#ifndef OFFBYNULL_UTILS_H
#define OFFBYNULL_UTILS_H

#include <cstddef>
#include <concepts>
#include <vector>
#include <set>
#include <ranges>
#include <type_traits>
#include <optional>
#include <utility>
#include <random>
#include <string>
#include <boost/container/static_vector.hpp>
#include <boost/container/options.hpp>
#include "offbynull/concepts.h"

// Adapted from https://stackoverflow.com/a/3312896
#if defined(OBN_PACK_STRUCTS)
#if defined(__GNUC__) || defined(__INTEL_COMPILER)  // G++, Clang, or Intel C++ Compiler
#define PACK_STRUCT_START
#define PACK_STRUCT_STOP __attribute__((__packed__))
#elif defined(_MSC_VER)  // MSVC
#define PACK_STRUCT_START __pragma(pack(push, 1))
#define PACK_STRUCT_STOP __pragma(pack(pop))
#else
static_assert(false, "Struct packing not supported by compiler. Turn off OBN_PACK_STRUCTS define.");
#endif
#else
#define PACK_STRUCT_START
#define PACK_STRUCT_STOP
#endif

namespace offbynull::utils {
    using offbynull::concepts::unqualified_value_type;

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
        constexpr static T V0 { static_cast<T>(0) };
        constexpr static T V1 { static_cast<T>(1) };
    };

    auto copy_to_vector(std::ranges::range auto&& range) -> std::vector<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::vector<ELEM> ret {};
        for (const auto& e : range) {
            ret.push_back(e);
        }
        return ret;
    }

    auto copy_to_set(std::ranges::range auto&& range) -> std::set<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::set<ELEM> ret {};
        for (const auto& e : range) {
            ret.insert(e);
        }
        return ret;
    }

    auto copy_to_multiset(std::ranges::range auto&& range) -> std::multiset<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::multiset<ELEM> ret {};
        for (const auto& e : range) {
            ret.insert(e);
        }
        return ret;
    }

    PACK_STRUCT_START
    template<std::regular T>
    struct packable_optional {
        bool exists;
        T value;

        packable_optional()
        : exists{ false }
        , value {} {}

        packable_optional(const T& value_)
        : exists { true }
        , value { value_ } {}

        packable_optional(T&& value_)
        : exists { true }
        , value { std::move(value_) } {}

        packable_optional(std::nullopt_t)
        : exists { false }
        , value {} {}

        bool has_value() const {
            return exists;
        }

        T& operator*() {
            return value;
        }

        const T& operator*() const {
            return value;
        }

        bool operator==(const packable_optional& other) const = default;
    }
    PACK_STRUCT_STOP;

    constexpr bool is_debug_mode() {
#ifdef NDEBUG
        return false;
#else
        return true;
#endif
    }

    template<std::integral T>
    T random_integer(std::mt19937_64& rand, T a, T b) {
        return std::uniform_int_distribution<decltype(a)>(a,b)(rand);
    }

    template<std::floating_point T>
    T random_float(std::mt19937_64& rand, T a, T b) {
        return std::uniform_real_distribution<decltype(a)>(a,b)(rand);
    }

    inline std::string random_printable_ascii(std::mt19937_64& rand, std::size_t len) {
        std::string ret {};
        ret.reserve(len);
        for (std::size_t i { 0zu }; i < len; ++i) {
            ret += random_integer<char>(rand, static_cast<char>(32), static_cast<char>(126));
        }
        return ret;
    }
}

#endif //OFFBYNULL_UTILS_H
