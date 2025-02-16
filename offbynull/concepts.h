#ifndef OFFBYNULL_CONCEPTS_H
#define OFFBYNULL_CONCEPTS_H

#include <ranges>
#include <limits>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <cstddef>

/**
 * Common concepts.
 */
namespace offbynull::concepts {
    /**
     * Concept that's satisfied if `T` is neither a void, nor a reference, nor const/volatile.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept unqualified_value_type = !std::is_void_v<T> && std::is_same_v<T, std::remove_cvref_t<T>>;

    /**
     * Concept that's satisfied if `T` is implicitly convertible to an @ref offbynull::concepts::unqualified_value_type.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept convertible_to_unqualified_value_type= !std::is_void_v<T> && std::is_convertible_v<T, std::remove_cvref_t<T>>;

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::random_access_range` and `T`'s element type matches `V`.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s element type.
     */
    template <typename T, typename V>
    concept random_access_range_of_type = std::ranges::random_access_range<T> && std::same_as<std::ranges::range_reference_t<T>, V&>;

    /**
     * Concept that's satisfied if `T` satisfies `std::input_iterator` and `T`'s element type matches `V`.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s element type.
     */
    template <typename T, typename V>
    concept input_iterator_of_type = std::input_iterator<T> && std::same_as<typename T::value_type, V>;

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::range` and `T`'s element type matches `V`.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s element type.
     */
    template<typename T, typename V>
    concept range_of_type = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, V>;

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::range` and `T`'s element type matches one of the types within `Vs`.
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type.
     */
    template<typename T, typename... Vs>
    concept range_of_one_of = std::ranges::range<T> && (std::same_as<std::ranges::range_value_t<T>, Vs> || ...);

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::range` and `T`'s element type satisfies `range_of_one_of<..., Vs>`.
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type's element type.
     */
    template<typename T, typename... Vs>
    concept range_of_range_of_one_of = std::ranges::range<T> && std::ranges::range<std::ranges::range_value_t<T>>
        && (std::same_as<std::ranges::range_value_t<std::ranges::range_value_t<T>>, Vs> || ...);

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s element type matches `V`.
     *
     * @tparam T Type to check.
     * @tparam V Type allowed for `T`'s element type.
     */
    template<typename T, typename V>
    concept bidirectional_range_of_type = std::ranges::bidirectional_range<T> && std::same_as<std::ranges::range_value_t<T>, V>;

    /**
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s element type matches one of the types within
     * `Vs`.
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type.
     */
    template<typename T, typename... Vs>
    concept bidirectional_range_of_one_of = std::ranges::bidirectional_range<T>
        && (std::same_as<std::ranges::range_value_t<T>, Vs> || ...);

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
     * Concept that's satisfied if `T` satisfies `std::ranges::bidirectional_range` and `T`'s element type satisfies
     * `bidirectional_range_of_one_of<..., Vs>`.
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type's element type.
     */
    template<typename T, typename... Vs>
    concept bidirectional_range_of_bidirectional_range_of_one_of = std::ranges::bidirectional_range<T>
        && std::ranges::bidirectional_range<std::ranges::range_value_t<T>>
        && (std::same_as<std::ranges::range_value_t<std::ranges::range_value_t<T>>, Vs> || ...);

    /**
     * Concept that's satisfied if `T` matches one of the types within `Vs`.
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type.
     */
    template<typename T, typename ... Vs>
    concept one_of = (std::same_as<T, Vs> || ...);

    /**
     * Concept that's satisfied if `T` is can be cast to `std::size_t` without data loss / truncation.
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept widenable_to_size_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() <= std::numeric_limits<std::size_t>::max();

    /**
     * Concept that's satisfied if `T` is in same as all types in `Vs` (each type `V` in `Vs` satisfies `std::same_as<T, V>`).
     *
     * @tparam T Type to check.
     * @tparam Vs Types allowed for `T`'s element type.
     */
    template<typename T, typename... Vs>
    concept many_same_as = (std::same_as<T, Vs> && ...);
}

#endif //OFFBYNULL_CONCEPTS_H
