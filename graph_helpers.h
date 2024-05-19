#ifndef GRAPH_HELPERS_H
#define GRAPH_HELPERS_H

#include <format>
#include <string>
#include "boost/container/small_vector.hpp"
#include "boost/container/static_vector.hpp"

namespace offbynull::graph::graph_helpers {
    template<typename ELEM, typename T, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class VectorAllocator {
    public:
        std::vector<ELEM> allocate(T down_node_cnt, T right_node_cnt) {
            return std::vector<ELEM>(down_node_cnt * right_node_cnt);
        }
    };

    template<typename ELEM, typename T, T STATIC_DOWN_CNT, T STATIC_RIGHT_CNT, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class ArrayAllocator {
    private:
        static constexpr T size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        std::array<ELEM, size> allocate(T down_node_cnt, T right_node_cnt) {
            if constexpr (error_check) {
                if (down_node_cnt != STATIC_DOWN_CNT || right_node_cnt != STATIC_RIGHT_CNT) {
                    throw std::runtime_error("Unexpected number of elements");
                }
            }
            return std::array<ELEM, size>{};
        }
    };

    template<typename ELEM, typename T, T STATIC_DOWN_CNT, T STATIC_RIGHT_CNT, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class StaticVectorAllocator {
    private:
        static constexpr T max_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        boost::container::static_vector<ELEM, max_size> allocate(T down_node_cnt, T right_node_cnt) {
            if constexpr (error_check) {
                if (down_node_cnt > STATIC_DOWN_CNT || right_node_cnt > STATIC_RIGHT_CNT) {
                    throw std::runtime_error("Too many elements");
                }
            }
            return boost::container::static_vector<ELEM, max_size>(down_node_cnt * right_node_cnt);
        }
    };

    template<typename ELEM, typename T, T STATIC_DOWN_CNT, T STATIC_RIGHT_CNT, bool error_check = true>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class SmallVectorAllocator {
    private:
        static constexpr T max_stack_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
    public:
        boost::container::small_vector<ELEM, max_stack_size> allocate(T down_node_cnt, T right_node_cnt) {
            return boost::container::small_vector<ELEM, max_stack_size>(down_node_cnt * right_node_cnt);
        }
    };






    std::string graph_to_string(const auto &g) {
        std::string out {};
        for (const auto& node : g.get_nodes()) {
            out += std::format("node {}: {}\n", node, g.get_node_data(node));
            for (const auto& edge : g.get_outputs(node)) {
                auto [from_node, to_node, edge_data] = g.get_edge(edge);
                out += std::format("  edge {} pointing to node {}: {}\n", edge, to_node, edge_data);
            }
        }
        return out;
    }
}

#endif //GRAPH_HELPERS_H
