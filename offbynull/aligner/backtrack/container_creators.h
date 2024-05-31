#ifndef OFFBYNULL_ALIGNER_BACKTRACK_CONTAINER_CREATORS_H
#define OFFBYNULL_ALIGNER_BACKTRACK_CONTAINER_CREATORS_H

#include <stdexcept>
#include <cstddef>
#include <vector>
#include "boost/container/small_vector.hpp"
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrack::container_creators {
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::utils::static_vector_typer;

    template<typename ELEM_, bool error_check = true>
    class vector_container_creator {
    public:
        using ELEM = ELEM_;

        std::vector<ELEM> create_empty(std::optional<std::size_t> capacity) {
            std::vector<ELEM> ret {};
            if (capacity.has_value()) {
                ret.reserve(capacity.value());
            }
            return ret;
        }

        std::vector<ELEM> create_objects(std::size_t cnt) {
            return std::vector<ELEM>(cnt);
        }

        std::vector<ELEM> create_copy(auto begin, auto end) {
            return std::vector<ELEM>(begin, end);
        }
    };
    static_assert(container_creator<vector_container_creator<int>>);  // Sanity check

    template<typename ELEM_, std::size_t size, bool error_check = true>
    class array_container_creator {
    public:
        using ELEM = ELEM_;

        std::array<ELEM, size> create_empty(std::optional<std::size_t> capacity) {
            static_assert(size != 0zu, "If calling this function, size must be non-zero");
            std::array<ELEM, size> ret{};
            return ret;
        }

        std::array<ELEM, size> create_objects(std::size_t cnt) {
            if constexpr (error_check) {
                if (cnt != size) {
                    throw std::runtime_error("Unexpected number of elements");
                }
            }
            return std::array<ELEM, size>{};
        }

        std::array<ELEM, size> create_copy(auto& begin, auto& end) {
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

        static_vector_typer<ELEM, max_size, error_check>::type create_empty(std::optional<std::size_t> capacity) {
            return typename static_vector_typer<ELEM, max_size, error_check>::type {};
        }

        static_vector_typer<ELEM, max_size, error_check>::type create_objects(std::size_t cnt) {
            if constexpr (error_check) {
                if (cnt > max_size) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return typename static_vector_typer<ELEM, max_size, error_check>::type(cnt);
        }

        static_vector_typer<ELEM, max_size, error_check>::type create_copy(auto& begin, auto& end) {
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

        boost::container::small_vector<ELEM, max_stack_size> create_empty(std::optional<std::size_t> capacity) {
            return boost::container::small_vector<ELEM, max_stack_size> {};
        }

        boost::container::small_vector<ELEM, max_stack_size> create_objects(std::size_t cnt) {
            return boost::container::small_vector<ELEM, max_stack_size>(cnt);
        }

        boost::container::small_vector<ELEM, max_stack_size> create_copy(auto& begin, auto& end) {
            return boost::container::small_vector<ELEM, max_stack_size>(begin, end);
        }
    };
    static_assert(container_creator<small_vector_container_creator<int, 0zu>>);  // Sanity check
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_CONTAINER_CREATORS_H
