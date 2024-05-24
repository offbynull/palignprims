#ifndef OFFBYNULL_ALIGNER_GRAPH_GRID_ALLOCATORS_H
#define OFFBYNULL_ALIGNER_GRAPH_GRID_ALLOCATORS_H

#include <vector>
#include <array>
#include "boost/container/small_vector.hpp"
#include "boost/container/static_vector.hpp"
#include "offbynull/aligner/graph/grid_allocator.h"


namespace offbynull::aligner::graph::grid_allocators {
    using offbynull::aligner::graph::grid_allocator::grid_allocator;

    template<typename ELEM_, typename T, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class VectorGridAllocator {
    public:
        using ELEM = ELEM_;
        std::vector<ELEM> allocate(T down_node_cnt, T right_node_cnt) {
            return std::vector<ELEM>(down_node_cnt * right_node_cnt);
        }
    };
    static_assert(grid_allocator<VectorGridAllocator<int, size_t>, size_t>);  // Sanity check

    template<typename ELEM_, typename T, T STATIC_DOWN_CNT, T STATIC_RIGHT_CNT, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class ArrayGridAllocator {
    private:
        static constexpr T size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        using ELEM = ELEM_;
        std::array<ELEM, size> allocate(T down_node_cnt, T right_node_cnt) {
            if constexpr (error_check) {
                if (down_node_cnt != STATIC_DOWN_CNT || right_node_cnt != STATIC_RIGHT_CNT) {
                    throw std::runtime_error("Unexpected number of elements");
                }
            }
            return std::array<ELEM, size>{};
        }
    };
    static_assert(grid_allocator<ArrayGridAllocator<int, size_t, 0u, 0u>, size_t>);  // Sanity check

    template<typename ELEM_, typename T, T STATIC_DOWN_CNT, T STATIC_RIGHT_CNT, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class StaticVectorGridAllocator {
    private:
        static constexpr T max_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        using ELEM = ELEM_;
        boost::container::static_vector<ELEM, max_size> allocate(T down_node_cnt, T right_node_cnt) {
            if constexpr (error_check) {
                if (down_node_cnt > STATIC_DOWN_CNT || right_node_cnt > STATIC_RIGHT_CNT) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return boost::container::static_vector<ELEM, max_size>(down_node_cnt * right_node_cnt);
        }
    };
    static_assert(grid_allocator<StaticVectorGridAllocator<int, size_t, 0u, 0u>, size_t>);  // Sanity check

    template<typename ELEM_, typename T, T STATIC_DOWN_CNT, T STATIC_RIGHT_CNT, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class SmallVectorGridAllocator {
    private:
        static constexpr T max_stack_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        using ELEM = ELEM_;
        boost::container::small_vector<ELEM, max_stack_size> allocate(T down_node_cnt, T right_node_cnt) {
            return boost::container::small_vector<ELEM, max_stack_size>(down_node_cnt * right_node_cnt);
        }
    };
    static_assert(grid_allocator<SmallVectorGridAllocator<int, size_t, 0u, 0u>, size_t>);  // Sanity check
}

#endif //OFFBYNULL_ALIGNER_GRAPH_GRID_ALLOCATORS_H
