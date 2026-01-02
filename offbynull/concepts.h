#ifndef OFFBYNULL_CONCEPTS_H
#define OFFBYNULL_CONCEPTS_H

#include <ranges>
#include <limits>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

namespace offbynull::concepts {
    /**
     * Concept that's satisfied if `T` matches one of the types within `Vs`.
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type.
     */
    template<typename T, typename ... Vs>
    concept one_of = (std::same_as<T, Vs> || ...);

    /**
     * Concept that's satisfied if `T` is neither a void, nor a reference, nor const/volatile.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept unqualified_object_type = !std::is_void_v<T> && std::is_same_v<T, std::remove_cvref_t<T>>;
    // TODO: Use std::is_object_v instead of !std::is_void_v? That way, you don't have to check for ref as well (a ref doesn't qualify as
    //       an object type) - you can just remove CV.

    /**
     * Concept that's satisfied if `T` is implicitly convertible to an @ref offbynull::concepts::unqualified_object_type.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept convertible_to_unqualified_object_type= !std::is_void_v<T> && std::is_convertible_v<T, std::remove_cvref_t<T>>;

    /**
     * Concept that's satisfied if `T` satisfies `std::input_iterator` and `T` dereferences to *exactly* `V` (not just convertible to
     * `V`).
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s element type.
     */
    template <typename T, typename V>
    concept input_iterator_of_exact = std::input_iterator<T> && std::same_as<std::iter_reference_t<T>, V>;
    // NOTE: It turns out that std::iter_reference_t returns whatever type the iterator deference operation returns. So, for example, if
    //       dereferencing returns int instead int&, std::iter_reference_t<T> will resolve to int.
    //
    // NOTE: std::iter_reference_t<T> works on a broader set of type than std::iterator_traits<T>::reference. When I used the
    //       std::iterator_traits<T>::reference, it would fail for some of my ranges (resolved to void?).

    /**
     * Concept that's satisfied if `T` satisfies `std::input_iterator` and `T` dereferences to `V` once const, volatile,
     * and references are removed.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T` dereference type (non-cvref).
     */
    template <typename T, typename V>
    concept input_iterator_of_non_cvref = std::input_iterator<T>
        && (unqualified_object_type<V> && std::same_as<std::remove_cvref_t<std::iter_reference_t<T>>, V>);

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::forward_range` and `T`'s iterator dereferences to *exactly* `V` (not just
     * convertible to `V`).
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type.
     */
    template<typename T, typename V>
    concept forward_range_of_exact = std::ranges::forward_range<T>
        && std::same_as<std::ranges::range_reference_t<T>, V>;
    // NOTE: It turns out that std::ranges::range_reference_t returns whatever type the range's iterator deference operation returns. For
    //       example, if dereferencing returns int instead int&, std::ranges::range_reference_t<T> will resolve to int.

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::forward_range` and `T`'s iterator dereferences to *exactly* one the types in
     * `Vs` (not just convertible to one of the types in `Vs`).
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s iterator dereference type.
     */
    template<typename T, typename... Vs>
    concept forward_range_of_one_of_exact = (forward_range_of_exact<T, Vs> || ...);

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::forward_range` and `T`'s iterator dereferences to `V` once const, volatile,
     * and references are removed.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type (non-cvref).
     */
    template<typename T, typename V>
    concept forward_range_of_non_cvref = std::ranges::forward_range<T>
        && (unqualified_object_type<V> && std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<T>>, V>);
    // NOTE: It turns out that std::ranges::range_reference_t returns whatever type the range's iterator deference operation returns. For
    //       example, if dereferencing returns int instead int&, std::ranges::range_reference_t<T> will resolve to int.

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s iterator dereferences to *exactly* `V` (not
     * just convertible to `V`).
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type.
     */
    template<typename T, typename V>
    concept bidirectional_range_of_exact = std::ranges::bidirectional_range<T>
        && std::same_as<std::ranges::range_reference_t<T>, V>;
    // NOTE: It turns out that std::ranges::range_reference_t returns whatever type the range's iterator deference operation returns. For
    //       example, if dereferencing returns int instead int&, std::ranges::range_reference_t<T> will resolve to int.

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s iterator dereferences to *exactly* one the
     * types in `Vs` (not just convertible to one of the types in `Vs`).
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s iterator dereference type.
     */
    template<typename T, typename... Vs>
    concept bidirectional_range_of_one_of_exact = (bidirectional_range_of_exact<T, Vs> || ...);

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s iterator dereferences to `V` once const,
     * volatile, and references are removed.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type (non-cvref).
     */
    template<typename T, typename V>
    concept bidirectional_range_of_non_cvref = std::ranges::bidirectional_range<T>
        && (unqualified_object_type<V> && std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<T>>, V>);
    // NOTE: It turns out that std::ranges::range_reference_t returns whatever type the range's iterator deference operation returns. For
    //       example, if dereferencing returns int instead int&, std::ranges::range_reference_t<T> will resolve to int.

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s element type satisfies
     * `std::ranges::bidirectional_range`.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept bidirectional_range_of_bidirectional_range = std::ranges::bidirectional_range<T>
        && std::ranges::bidirectional_range<std::ranges::range_value_t<T>>;

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::random_access_range` and `T`'s iterator dereferences to *exactly* `V` (not
     * just convertible to `V`).
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type.
     */
    template<typename T, typename V>
    concept random_access_range_of_exact = std::ranges::random_access_range<T>
        && std::same_as<std::ranges::range_reference_t<T>, V>;
    // NOTE: It turns out that std::ranges::range_reference_t returns whatever type the range's iterator deference operation returns. For
    //       example, if dereferencing returns int instead int&, std::ranges::range_reference_t<T> will resolve to int.

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::random_access_range` and `T`'s iterator dereferences to *exactly* one the
     * types in `Vs` (not just convertible to one of the types in `Vs`).
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s iterator dereference type.
     */
    template<typename T, typename... Vs>
    concept random_access_range_of_one_of_exact = (random_access_range_of_exact<T, Vs> || ...);

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::random_access_range` and `T`'s iterator dereferences to `V` once const,
     * volatile, and references are removed.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type (non-cvref).
     */
    template<typename T, typename V>
    concept random_access_range_of_non_cvref = std::ranges::random_access_range<T>
        && (unqualified_object_type<V> && std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<T>>, V>);
    // NOTE: It turns out that std::ranges::range_reference_t returns whatever type the range's iterator deference operation returns. For
    //       example, if dereferencing returns int instead int&, std::ranges::range_reference_t<T> will resolve to int.

    /**
     * Concepts that's satisfied if `T` is similar `std::vector, allowing subscript accessor/mutator, `size()`, and `begin()` / `end()`.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s iterator dereference type / `T`'s access type.
     */
    template<typename T, typename V>
    concept random_access_sequence_container =
        random_access_range_of_one_of_exact<T, V&, const V&> && std::ranges::sized_range<T>
        && requires(T r, V v, std::ranges::range_difference_t<T> i) {
            { r.begin() } -> std::random_access_iterator;
            { r.end() } -> std::sentinel_for<decltype(r.begin())>;
            { r.size() } -> std::convertible_to<std::size_t>;
            { r[i] } -> one_of<V&, const V&>;
            r[i] = v;
        };
    
    /**
     * Concept that's satisfied if `T` can be cast to `std::size_t` without data loss / truncation.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept widenable_to_size_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() <= std::numeric_limits<std::size_t>::max();

    /**
     * Concept that's satisfied if `T` is in same as `V` const, volatile, and references are removed.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T` (non-cvref).
     */
    template<typename T, typename V>
    concept same_as_non_cvref = unqualified_object_type<V> && std::same_as<std::remove_cvref_t<T>, V>;

    // TODO: Collapse the similar concepts below into a single concept that uses variadic template parameter? Getting the index to feed into
    //       std:get<>() seems like the tricky part.

    /**
     * Concept that's satisfied if `T` is a `std::tuple<>` with a single item matching `V0` once const, volatile, and references are
     * removed.
     *
     * @tparam T Type to check.
     * @tparam V0 Type 1 in tuple (non-cvref).
     */
    template<typename T, typename V0>
    concept tuple_with_1_non_cvref_children = unqualified_object_type<T>
        && std::tuple_size_v<T> == 1zu
        && (unqualified_object_type<V0> && std::same_as<std::remove_cvref_t<decltype(std::get<0zu>(std::declval<T>()))>, V0>);

    /**
     * Concept that's satisfied if `T` is a `std::tuple<>` with 2 items matching [`V0`, `V1`] once const, volatile, and references are
     * removed.
     *
     * @tparam T Type to check.
     * @tparam V0 Type 1 in tuple (non-cvref).
     * @tparam V1 Type 2 in tuple (non-cvref).
     */
    template<typename T, typename V0, typename V1>
    concept tuple_with_2_non_cvref_children = unqualified_object_type<T>
        && std::tuple_size_v<T> == 2zu
        && (unqualified_object_type<V0> && std::same_as<std::remove_cvref_t<decltype(std::get<0zu>(std::declval<T>()))>, V0>)
        && (unqualified_object_type<V1> && std::same_as<std::remove_cvref_t<decltype(std::get<1zu>(std::declval<T>()))>, V1>);

    /**
     * Concept that's satisfied if `T` is a `std::tuple<>` with 3 items matching [`V0`, `V1`, `V2`] once const, volatile, and references are
     * removed.
     *
     * @tparam T Type to check.
     * @tparam V0 Type 1 in tuple (non-cvref).
     * @tparam V1 Type 2 in tuple (non-cvref).
     * @tparam V2 Type 3 in tuple (non-cvref).
     */
    template<typename T, typename V0, typename V1, typename V2>
    concept tuple_with_3_non_cvref_children = unqualified_object_type<T>
        && std::tuple_size_v<T> == 3zu
        && (unqualified_object_type<V0> && std::same_as<std::remove_cvref_t<decltype(std::get<0zu>(std::declval<T>()))>, V0>)
        && (unqualified_object_type<V1> && std::same_as<std::remove_cvref_t<decltype(std::get<1zu>(std::declval<T>()))>, V1>)
        && (unqualified_object_type<V2> && std::same_as<std::remove_cvref_t<decltype(std::get<2zu>(std::declval<T>()))>, V2>);

    /**
     * Concept that's satisfied if `T` is a `std::tuple<>` with 3 items matching [`V0`, `V1`, `V2`, `V3`] once const, volatile, and
     * references are removed.
     *
     * @tparam T Type to check.
     * @tparam V0 Type 1 in tuple (non-cvref).
     * @tparam V1 Type 2 in tuple (non-cvref).
     * @tparam V2 Type 3 in tuple (non-cvref).
     * @tparam V3 Type 4 in tuple (non-cvref).
     */
    template<typename T, typename V0, typename V1, typename V2, typename V3>
    concept tuple_with_4_non_cvref_children = unqualified_object_type<T>
        && std::tuple_size_v<T> == 4zu
        && (unqualified_object_type<V0> && std::same_as<std::remove_cvref_t<decltype(std::get<0zu>(std::declval<T>()))>, V0>)
        && (unqualified_object_type<V1> && std::same_as<std::remove_cvref_t<decltype(std::get<1zu>(std::declval<T>()))>, V1>)
        && (unqualified_object_type<V2> && std::same_as<std::remove_cvref_t<decltype(std::get<2zu>(std::declval<T>()))>, V2>)
        && (unqualified_object_type<V3> && std::same_as<std::remove_cvref_t<decltype(std::get<3zu>(std::declval<T>()))>, V3>);


    /**
     * Concept that's satisfied is `T` is an integer type, including extended integer types that would normally return false on
     * std::integral (e.g., __int128).
     */
    template<typename T>
    concept any_integer = std::numeric_limits<T>::is_integer || std::integral<T>;

    /**
     * Concept that's satisfied if `T` is either an integer or a floating point number.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept numeric = std::floating_point<T> || any_integer<T>;
}

#endif //OFFBYNULL_CONCEPTS_H
