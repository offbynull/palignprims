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
#include <array>
#include <stdexcept>
#include <limits>
#include <iterator>
#include <boost/container/static_vector.hpp>
#include <boost/container/options.hpp>
#include <boost/safe_numerics/checked_default.hpp>
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
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Unimplemented class template used as a hack to determine what some unknown type `T` is. Because this class template remains
     * unimplemented for every `T`, declaring it a variable of type `type_displayer<T>` will result in a compiler error that explicitly
     * states what `T` is.
     *
     * ```cpp
     * void my_func(const auto& x) {
     *     type_displayer<decltype(x)> unused;  // error: 'type_displayer<const int&> unused' has incomplete type
     * }
     * ```
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
     * @tparam debug_mode Vector debug features enabled if `true`, otherwise disables them.
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
                boost::container::inplace_alignment<0zu>
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

    // NOTE: The following copy functions exist because, when _GLIBCXX_DEBUG, the standard library fails to create common containers (e.g.,
    //       std::set or std::vector) when copying from some newer types of ranges and iterators (e.g., range consisting of
    //       cartesian_product_view -> transform_view). I don't know the full story behind why this is. It may just be that some parts of
    //       the standard library haven't been fully implemented yet with my version of g++/libstdc++ (g++ version 14.2).
    //
    //       If this weren't an issue, instead of copy_to_vector(r), you could do directly do std::vector(r).

    /**
     * Copy / move range into `std::vector`, where each element of the vector is a copy of each element in the range.
     *
     * @param range Range to copy from.
     * @return `std::vector` containing copies of the elements in `range`.
     */
    auto copy_to_vector(std::ranges::range auto&& range) -> std::vector<std::remove_cvref_t<decltype(*range.begin())>> {
        using ELEM = std::remove_cvref_t<decltype(*range.begin())>;
        std::vector<ELEM> ret(0zu);
        if constexpr (std::ranges::sized_range<decltype(range)>) {
            ret.reserve(std::ranges::size(range));
        }
        for (auto&& e : range) {
            ret.emplace_back(std::forward<decltype(e)>(e));
        }
        return ret;
    }

    /**
     * Copy / move range into `std::vector`, where each element of the vector is a copy of each element in the range.
     *
     * If `begin` and `end` aren't for the same range, the behavior of this function is undefined.
     *
     * @param begin Starting iterator to copy / move from.
     * @param end Ending iterator to copy / move from.
     * @return `std::vector` containing copies of the elements in `begin` to `end`.
     */
    auto copy_to_vector(
        std::input_iterator auto begin,
        std::sentinel_for<decltype(begin)> auto end
        // NOTE: Do not use (std::input_iterator auto&& begin, std::sentinel_for<decltype(begin)> auto&& end). It won't work because these
        //       concepts require std::movable, which require that they be object types (not references)? Copies should be cheap and each
        //       time you pass an iterator around it should be a copy (so that if it gets mutated it doesn't destroy the original)?
        //
        //       If you want to support &&, do ...
        //
        //               template<typename I, typename S>
        //               requires std::input_iterator<std::remove_reference_t<I>>
        //                   && std::sentinel_for<std::remove_reference_t<S>, std::remove_reference_t<I>>
        //               auto copy_to_vector(I&& begin, S&& end) { ... }
    ) -> std::vector<std::remove_cvref_t<decltype(*begin)>> {
        using ELEM = std::remove_cvref_t<decltype(*begin)>;
        std::vector<ELEM> ret(0zu);
        if constexpr (std::sized_sentinel_for<decltype(begin), decltype(end)>) {
            auto dist { end - begin };
            ret.reserve(static_cast<std::size_t>(dist));
        }
        while (begin != end) {
            ret.emplace_back(*begin);
            ++begin;
        }
        return ret;
    }

    /**
     * Copy / move range into `std::set`, where each element of the set is a copy of each element in the range.
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
     * Copy / move range into `std::multiset`, where each element of the multiset is a copy of each element in the range.
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
     * Analog to `std::optional`, but memory layout is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct
     * packing).
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

    /**
     * Throw exception if a multiplication chain will result in an overflow. The widest possible type is assumed to be std::size_t.
     *
     * @tparam R Result type (must be wide enough to hold the multiplication's output).
     * @tparam A0 First operands type.
     * @tparam As Remaining operands types (must all be the same as `A0`).
     * @param rest Operands.
     * @throws std::runtime_error If `operands` multiply to a type that's too wide to be held in `R`.
     */
    template<widenable_to_size_t R, widenable_to_size_t A0, widenable_to_size_t... As>
        requires (std::is_same_v<As, A0> && ...)
    constexpr void check_multiplication_nonoverflow(const A0 first, const As... rest) {
        std::size_t max_size { first };
        std::array<A0, sizeof...(rest)> operands_ { rest... };
        for (const auto op : operands_) {
            auto res { boost::safe_numerics::checked::multiply<std::size_t>(max_size, op) };
            if (res.exception()) {
                throw std::runtime_error { "Type not wide enough" };
            }
            max_size = static_cast<std::size_t>(res);
        }
        if (std::numeric_limits<R>::max() <= max_size) {
            throw std::runtime_error { "Type not wide enough" };
        }
    }
}

#endif //OFFBYNULL_UTILS_H
