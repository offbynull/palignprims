#ifndef OFFBYNULL_CONCEPTS_H
#define OFFBYNULL_CONCEPTS_H

#include <ranges>
#include <limits>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <cstddef>

namespace offbynull::concepts {
    template<typename T>
    concept unqualified_value_type = !std::is_void_v<T> && std::is_same_v<T, std::remove_cvref_t<T>>;

    template<typename T>
    concept convertible_to_unqualified_value_type= !std::is_void_v<T> && std::is_convertible_v<T, std::remove_cvref_t<T>>;

    template <typename T, typename V>
    concept random_access_range_of_type = std::ranges::random_access_range<T> && std::same_as<std::ranges::range_reference_t<T>, V&>;

    template <typename T, typename V>
    concept input_iterator_of_type = std::input_iterator<T> && std::same_as<typename T::value_type, V>;

    template<typename T, typename V>
    concept range_of_type = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, V>;

    template<typename T, typename... Ts>
    concept range_of_one_of = std::ranges::range<T> && (std::same_as<std::ranges::range_value_t<T>, Ts> || ...);

    template<typename T, typename ... Ts>
    concept one_of = (std::same_as<T, Ts> || ...);

    // Check T can be widened to size_t, but not narrowed (to avoid data loss bugs). Is this a valid test? Things can be padded, meaning
    // the sizeof() test below could be wrong.
    template<typename T>
    concept widenable_to_size_t = std::unsigned_integral<T> && std::numeric_limits<T>::max() <= std::numeric_limits<std::size_t>::max();
}

#endif //OFFBYNULL_CONCEPTS_H
