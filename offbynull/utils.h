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

/**
 * Utility / helper functions.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::utils {
    using offbynull::concepts::unqualified_object_type;

    /**
     * Unimplemented class template used as a hack to determine what some unknown type `T` is. Because this class template remains
     * unimplemented for every `T`, declaring it a variable of type `type_displayer<T>` will result in a compiler error that explicitly
     * states what `T` is.
     *
     * @code{.cpp}
     * void my_func(const auto& x) {
     *     type_displayer<decltype(x)> unused;  // error: 'type_displayer<const int&> unused' has incomplete type
     * }
     * @endcode
     *
     * @tparam T Type to display.
     */
    template<typename T>
    struct type_displayer;

    /**
     * Unimplemented class template, where specializations are expected to declare a type alias for
     * `boost::container::static_vector<ELEM, cnt>` under the name `type`. The `debug_mode` parameter controls whether the resulting type
     * has debug features enabled or disabled (e.g. bounds-checking).
     *
     * @tparam debug_mode Vector debug features enabled if \c true, otherwise disables them.
     * @tparam ELEM Type of element held by vector.
     * @tparam cnt Maximum number of elements vector can hold.
     */
    template<bool debug_mode, typename ELEM, std::size_t cnt>
    struct static_vector_typer;

    /**
     * Template specialization for @ref static_vector_typer where `debug_mode` is enabled. The type alias for
     * `boost::container::static_vector<ELEM, cnt>` will be created with its defaults, which enables bounds checking.
     *
     * @tparam ELEM Type of element held by vector.
     * @tparam cnt Maximum number of elements vector can hold.
     */
    template<typename ELEM, std::size_t cnt>
    struct static_vector_typer<true, ELEM, cnt> {
        /**
         * Type alias for `boost::container::static_vector<ELEM, cnt>` with bounds checking enabled.
         */
        using type = boost::container::static_vector<ELEM, cnt>;
    };

    /**
     * Template specialization for @ref static_vector_typer where `debug_mode` is disabled. The type alias for
     * `boost::container::static_vector<ELEM, cnt>` will be created with bounds checking disabled.
     *
     * @tparam ELEM Type of element held by vector.
     * @tparam cnt Maximum number of elements vector can hold.
     */
    template<typename ELEM, std::size_t cnt>
    struct static_vector_typer<false, ELEM, cnt> {
        /**
         * Type alias for `boost::container::static_vector<ELEM, cnt>` with bounds checking disabled.
         */
        using type = boost::container::static_vector<
            ELEM,
            cnt,
            boost::container::static_vector_options<
                boost::container::throw_on_overflow<false>,
                boost::container::inplace_alignment<0u>
            >::type
        >;
    };

    /**
     * Concept that's satisfied if `T` is either an integral type or a floating point type.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept integral_or_floating_point = std::integral<T> || std::floating_point<T>;

    /**
     * Common constants for an integral / floating point type.
     *
     * @tparam T Integral or floating point type.
     */
    template<integral_or_floating_point T>
    struct constants {
        /** Constant 0. */
        constexpr static T V0 { static_cast<T>(0) };
        /** Constant 1. */
        constexpr static T V1 { static_cast<T>(1) };
    };

    /**
     * Copy range into `std::vector`, where each element of the vector is a copy of each element in the range.
     *
     * @param range Range to copy from.
     * @return `std::vector` containing copies of the elements in `range`.
     */
    auto copy_to_vector(std::ranges::range auto&& range) -> std::vector<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::vector<ELEM> ret {};
        for (const auto& e : range) {
            ret.push_back(e);
        }
        return ret;
    }

    /**
     * Copy range into `std::set`, where each element of the set is a copy of each element in the range.
     *
     * @param range Range to copy from.
     * @return `std::set` containing copies of the elements in `range`.
     */
    auto copy_to_set(std::ranges::range auto&& range) -> std::set<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::set<ELEM> ret {};
        for (const auto& e : range) {
            ret.insert(e);
        }
        return ret;
    }

    /**
     * Copy range into `std::multiset`, where each element of the multiset is a copy of each element in the range.
     *
     * @param range Range to copy from.
     * @return `std::multiset` containing copies of the elements in `range`.
     */
    auto copy_to_multiset(std::ranges::range auto&& range) -> std::multiset<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::multiset<ELEM> ret {};
        for (const auto& e : range) {
            ret.insert(e);
        }
        return ret;
    }

    PACK_STRUCT_START
    /**
     * Analog to `std::optional`, but memory layout is packed when `OBN_PACK_STRUCTS` macro is defined.
     *
     * @tparam T Type to hold.
     */
    template<std::regular T>
    struct packable_optional {
    private:
        bool exists;
        T value;

    public:
        /**
         * Construct an @ref offbynull::utils::packable_optional instance that holds nothing.
         */
        packable_optional()
        : exists{ false }
        , value {} {}

        /**
         * Construct an @ref offbynull::utils::packable_optional instance that holds a copied value.
         *
         * @param value_ Value to copy.
         */
        packable_optional(const T& value_)
        : exists { true }
        , value { value_ } {}

        /**
         * Construct an @ref offbynull::utils::packable_optional instance that holds a moved value.
         *
         * @param value_ Value to move.
         */
        packable_optional(T&& value_)
        : exists { true }
        , value { std::move(value_) } {}

        /**
         * Construct an @ref offbynull::utils::packable_optional instance that holds nothing.
         */
        packable_optional(std::nullopt_t)
        : exists { false }
        , value {} {}

        /**
         * Test if value is held.
         *
         * @return True if value is held, or false if no value is held.
         */
        bool has_value() const {
            return exists;
        }

        /**
         * Get reference to held value.
         *
         * @return Held value, or dummy object if no value is held.
         */
        T& operator*() {
            return value;
        }

        /**
         * Get reference to held value (as const).
         *
         * @return Held value, or dummy object if no value is held.
         */
        const T& operator*() const {
            return value;
        }

        /**
         * Equality test.
         *
         * @param other Object to test against.
         * @return True if this object is equal to `other`, otherwise false.
         */
        bool operator==(const packable_optional& other) const = default;
    }
    PACK_STRUCT_STOP;

    /**
     * Test if `NDEBUG` macro is not defined.
     *
     * @return False if `NDEBUG` macro is defined, otherwise true.
     */
    constexpr bool is_debug_mode() {
#ifdef NDEBUG
        return false;
#else
        return true;
#endif
    }

    /**
     * Generate a random integer (uniform distribution).
     *
     * @tparam T Integer type to generate.
     * @param rand Random number engine.
     * @param a Lower-bound (inclusive).
     * @param b Upper-bound (inclusive).
     * @return Random integer between `[a, b]`.
     */
    template<std::integral T>
    T random_integer(std::mt19937_64& rand, T a, T b) {
        return std::uniform_int_distribution<decltype(a)>(a,b)(rand);
    }

    /**
     * Generate a random floating-point number (uniform distribution).
     *
     * @tparam T Floating-point type to generate.
     * @param rand Random number engine.
     * @param a Lower-bound (inclusive).
     * @param b Upper-bound (inclusive).
     * @return Random floating-point number between `[a, b]`.
     */
    template<std::floating_point T>
    T random_float(std::mt19937_64& rand, T a, T b) {
        return std::uniform_real_distribution<decltype(a)>(a,b)(rand);
    }

    /**
     * Generate a random ASCII string (uniform distribution).
     *
     * @param rand Random number engine.
     * @param len Length of string to generate.
     * @return Random ASCII string with `len` characters.
     */
    inline std::string random_printable_ascii(std::mt19937_64& rand, std::size_t len) {
        std::string ret {};
        ret.reserve(len);
        for (std::size_t i { 0zu }; i < len; ++i) {
            ret += random_integer<char>(rand, static_cast<char>(32), static_cast<char>(126));
        }
        return ret;
    }

    // A variant of the below struct was recommended by ChatGPT after asking how to enforce that member variables are available at
    // compile-time. It had originally recommended std::integral_constant for integers, but I wanted something that'd work for more than
    // just integers.
    /**
     * Wraps a compile-time value. Useful for use within concepts as a roundabout way of enforcing constexpr member variables. For example,
     * the following concept ensures that the member variable \c var is a constexpr:
     *
     * @code
     * template<typename T>
     * requires requires (T t) {
     *     compile_time_constant<t.var> {};
     * }
     * @endcode
     *
     * @tparam V Value.
     */
    template<auto V>
    struct compile_time_constant {
        using value_type = decltype(V);
        static constexpr value_type value { V };  // Can this be removed? Only having the line above should be good enough?
    };
}

#endif //OFFBYNULL_UTILS_H
