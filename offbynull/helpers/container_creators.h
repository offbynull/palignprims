#ifndef OFFBYNULL_HELPERS_CONTAINER_CREATORS_H
#define OFFBYNULL_HELPERS_CONTAINER_CREATORS_H

#include <stdexcept>
#include <cstddef>
#include <vector>
#include <ranges>
#include "boost/container/small_vector.hpp"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::helpers::container_creators {
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::utils::static_vector_typer;

    template <typename T>
    concept container_creator =
        requires(
            T t,
            std::size_t size,
            std::optional<std::size_t> capacity,
            std::vector<typename T::ELEM> r,
            decltype(std::declval<std::vector<typename T::ELEM>>().begin()) it
        ) {
            typename T::ELEM;
            { t.create_empty(capacity) } -> random_access_range_of_type<typename T::ELEM>;
            { t.create_objects(size) } -> random_access_range_of_type<typename T::ELEM>;
            { t.create_copy(r) } -> random_access_range_of_type<typename T::ELEM>;
            { t.create_copy(it, it) } -> random_access_range_of_type<typename T::ELEM>;
        };

    template <typename T, typename ELEM>
    concept container_creator_of_type =
        container_creator<T> && std::is_same_v<typename T::ELEM, ELEM>;

    template<typename ELEM_, bool error_check = true>
    class vector_container_creator {
    public:
        using ELEM = ELEM_;

        std::vector<ELEM> create_empty(std::optional<std::size_t> capacity) const {
            std::vector<ELEM> ret {};
            if (capacity.has_value()) {
                ret.reserve(capacity.value());
            }
            return ret;
        }

        std::vector<ELEM> create_objects(std::size_t cnt) const {
            return std::vector<ELEM>(cnt);
        }

        std::vector<ELEM> create_copy(const std::ranges::range auto& range) const {
            return create_copy(range.begin(), range.end());
        }

        std::vector<ELEM> create_copy(auto begin, auto end) const {
            return std::vector<ELEM>(begin, end);
        }
    };
    static_assert(container_creator<vector_container_creator<int>>);  // Sanity check

    template<typename ELEM_, std::size_t size, bool error_check = true>
    class array_container_creator {
    public:
        using ELEM = ELEM_;

        std::array<ELEM, size> create_empty(std::optional<std::size_t> capacity) const {
            static_assert(size != 0zu, "If calling this function, size must be non-zero");
            std::array<ELEM, size> ret{};
            return ret;
        }

        std::array<ELEM, size> create_objects(std::size_t cnt) const {
            if constexpr (error_check) {
                if (cnt != size) {
                    throw std::runtime_error("Unexpected number of elements");
                }
            }
            return std::array<ELEM, size>{};
        }

        std::array<ELEM, size> create_copy(const std::ranges::range auto& range) const {
            return create_copy(range.begin(), range.end());
        }

        std::array<ELEM, size> create_copy(auto& begin, auto& end) const {
            std::array<ELEM, size> ret;
            if constexpr (error_check) {
                auto it { begin };
                std::size_t cnt {};
                while (it != end) {
                    ret[cnt] = *it;
                    ++it;
                    ++cnt;
                }
                if (cnt != size) {
                    throw std::runtime_error("Unexpected number of elements");
                }
                return ret;
            } else {
                std::copy(begin, end, ret.begin());
            }
            return ret;
        }
    };
    static_assert(container_creator<array_container_creator<int, 0zu>>);  // Sanity check

    template<typename ELEM_, std::size_t max_size, bool error_check = true>
    class static_vector_container_creator {
    public:
        using ELEM = ELEM_;

        static_vector_typer<ELEM, max_size, error_check>::type create_empty(std::optional<std::size_t> capacity) const {
            return typename static_vector_typer<ELEM, max_size, error_check>::type {};
        }

        static_vector_typer<ELEM, max_size, error_check>::type create_objects(std::size_t cnt) const {
            if constexpr (error_check) {
                if (cnt > max_size) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return typename static_vector_typer<ELEM, max_size, error_check>::type(cnt);
        }

        static_vector_typer<ELEM, max_size, error_check>::type create_copy(const std::ranges::range auto& range) const {
            return create_copy(range.begin(), range.end());
        }

        static_vector_typer<ELEM, max_size, error_check>::type create_copy(auto& begin, auto& end) const {
            if constexpr (error_check) {
                auto cnt { end - begin };
                if (cnt > max_size) {
                    throw std::runtime_error("Too many elements");
                }
            }
            // In the signature, if I set the return type to auto, I get a bunch of concept check errors? So instead I
            // set it to the actual return type.
            return typename static_vector_typer<ELEM, max_size, error_check>::type(begin, end);
        }
    };
    static_assert(container_creator<static_vector_container_creator<int, 0zu>>);  // Sanity check

    template<typename ELEM_, std::size_t max_stack_size, bool error_check = true>
    class small_vector_container_creator {
    public:
        using ELEM = ELEM_;

        boost::container::small_vector<ELEM, max_stack_size> create_empty(std::optional<std::size_t> capacity) const {
            return boost::container::small_vector<ELEM, max_stack_size> {};
        }

        boost::container::small_vector<ELEM, max_stack_size> create_objects(std::size_t cnt) const {
            return boost::container::small_vector<ELEM, max_stack_size>(cnt);
        }

        boost::container::small_vector<ELEM, max_stack_size> create_copy(const std::ranges::range auto& range) const {
            return create_copy(range.begin(), range.end());
        }

        boost::container::small_vector<ELEM, max_stack_size> create_copy(auto& begin, auto& end) const {
            return boost::container::small_vector<ELEM, max_stack_size>(begin, end);
        }
    };
    static_assert(container_creator<small_vector_container_creator<int, 0zu>>);  // Sanity check
}

#endif //OFFBYNULL_HELPERS_CONTAINER_CREATORS_H
