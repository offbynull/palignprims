#ifndef OFFBYNULL_ALIGNER_GRAPH_GRID_CONTAINER_CREATORS_H
#define OFFBYNULL_ALIGNER_GRAPH_GRID_CONTAINER_CREATORS_H

#include <concepts>
#include <vector>
#include <array>
#include "boost/container/small_vector.hpp"
#include "boost/container/static_vector.hpp"
#include "offbynull/aligner/graph/grid_container_creator.h"


namespace offbynull::aligner::graph::grid_container_creators {
    using offbynull::aligner::graph::grid_container_creator::grid_container_creator;

    template<
        typename ELEM_,
        std::unsigned_integral INDEX,
        bool error_check = true
    >
    class vector_grid_container_creator {
    public:
        using ELEM = ELEM_;
        std::vector<ELEM> create_objects(INDEX down_node_cnt, INDEX right_node_cnt) {
            return std::vector<ELEM>(down_node_cnt * right_node_cnt);
        }
    };
    static_assert(grid_container_creator<vector_grid_container_creator<int, size_t>, size_t>);  // Sanity check

    template<
        typename ELEM_,
        std::unsigned_integral INDEX,
        INDEX STATIC_DOWN_CNT,
        INDEX STATIC_RIGHT_CNT,
        bool error_check = true
    >
    class array_grid_container_creator {
    private:
        static constexpr INDEX size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        using ELEM = ELEM_;
        std::array<ELEM, size> create_objects(INDEX down_node_cnt, INDEX right_node_cnt) {
            if constexpr (error_check) {
                if (down_node_cnt != STATIC_DOWN_CNT || right_node_cnt != STATIC_RIGHT_CNT) {
                    throw std::runtime_error("Unexpected number of elements");
                }
            }
            return std::array<ELEM, size>{};
        }
    };
    static_assert(grid_container_creator<array_grid_container_creator<int, size_t, 0u, 0u>, size_t>);  // Sanity check

    template<
        typename ELEM_,
        std::unsigned_integral INDEX,
        INDEX STATIC_DOWN_CNT,
        INDEX STATIC_RIGHT_CNT,
        bool error_check = true
    >
    class static_vector_grid_container_creator {
    private:
        static constexpr INDEX max_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        using ELEM = ELEM_;
        boost::container::static_vector<ELEM, max_size> create_objects(INDEX down_node_cnt, INDEX right_node_cnt) {
            if constexpr (error_check) {
                if (down_node_cnt > STATIC_DOWN_CNT || right_node_cnt > STATIC_RIGHT_CNT) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return boost::container::static_vector<ELEM, max_size>(down_node_cnt * right_node_cnt);
        }
    };
    static_assert(grid_container_creator<static_vector_grid_container_creator<int, size_t, 0u, 0u>, size_t>);  // Sanity check

    template<
        typename ELEM_,
        std::unsigned_integral INDEX,
        INDEX STATIC_DOWN_CNT,
        INDEX STATIC_RIGHT_CNT,
        bool error_check = true
>
    class small_vector_grid_container_creator {
    private:
        static constexpr INDEX max_stack_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        using ELEM = ELEM_;
        boost::container::small_vector<ELEM, max_stack_size> create_objects(INDEX down_node_cnt, INDEX right_node_cnt) {
            return boost::container::small_vector<ELEM, max_stack_size>(down_node_cnt * right_node_cnt);
        }
    };
    static_assert(grid_container_creator<small_vector_grid_container_creator<int, size_t, 0u, 0u>, size_t>);  // Sanity check
}

#endif //OFFBYNULL_ALIGNER_GRAPH_GRID_CONTAINER_CREATORS_H
