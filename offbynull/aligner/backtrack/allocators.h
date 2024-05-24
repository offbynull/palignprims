#ifndef OFFBYNULL_ALIGNER_BACKTRACK_ALLOCATORS_H
#define OFFBYNULL_ALIGNER_BACKTRACK_ALLOCATORS_H

#include <stdexcept>
#include <cstddef>
#include <vector>
#include "boost/container/small_vector.hpp"
#include "boost/container/static_vector.hpp"
#include "offbynull/aligner/backtrack/allocator.h"

namespace offbynull::aligner::backtrack::allocators {
    using offbynull::aligner::backtrack::allocator::allocator;

    template<typename ELEM_, bool error_check = true>
    class VectorAllocator {
    public:
        using ELEM = ELEM_;

        std::vector<ELEM> allocate(size_t cnt) {
            return std::vector<ELEM>(cnt);
        }

        std::vector<ELEM> allocate(auto& begin, auto& end) {
            return std::vector<ELEM>(begin, end);
        }
    };
    static_assert(allocator<VectorAllocator<int>>);  // Sanity check

    template<typename ELEM_, size_t size, bool error_check = true>
    class ArrayAllocator {
    public:
        using ELEM = ELEM_;

        std::array<ELEM, size> allocate(size_t cnt) {
            if constexpr (error_check) {
                if (cnt != size) {
                    throw std::runtime_error("Unexpected number of elements");
                }
            }
            return std::array<ELEM, size>{};
        }

        std::array<ELEM, size> allocate(auto& begin, auto& end) {
            std::array<ELEM, size> ret;
            if constexpr (error_check) {
                auto it { begin };
                size_t cnt {};
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
    static_assert(allocator<ArrayAllocator<int, 0u>>);  // Sanity check

    template<typename ELEM_, size_t max_size, bool error_check = true>
    class StaticVectorAllocator {
    public:
        using ELEM = ELEM_;

        boost::container::static_vector<ELEM, max_size> allocate(size_t cnt) {
            if constexpr (error_check) {
                if (cnt > max_size) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return boost::container::static_vector<ELEM, max_size>();
        }

        boost::container::static_vector<ELEM, max_size> allocate(auto& begin, auto& end) {
            if constexpr (error_check) {
                auto cnt { end - begin };
                if (cnt > max_size) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return boost::container::static_vector<ELEM, max_size>(begin, end);
        }
    };
    static_assert(allocator<StaticVectorAllocator<int, 0u>>);  // Sanity check

    template<typename ELEM_, size_t max_stack_size, bool error_check = true>
    class SmallVectorAllocator {
    public:
        using ELEM = ELEM_;

        boost::container::small_vector<ELEM, max_stack_size> allocate(size_t cnt) {
            return boost::container::small_vector<ELEM, max_stack_size>();
        }

        boost::container::small_vector<ELEM, max_stack_size> allocate(auto& begin, auto& end) {
            return boost::container::small_vector<ELEM, max_stack_size>(begin, end);
        }
    };
    static_assert(allocator<SmallVectorAllocator<int, 0u>>);  // Sanity check
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_ALLOCATORS_H
