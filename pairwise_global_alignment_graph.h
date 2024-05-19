#ifndef PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
#define PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H

#include <ranges>
#include <tuple>
#include <stdexcept>
#include <format>
#include <utility>
#include <functional>
#include <type_traits>
#include "grid_graph.h"

namespace offbynull::pairwise_aligner::global {
    template<
        typename _ND,
        typename _ED,
        typename T = unsigned int,
        typename _ND_ALLOCATOR = offbynull::graph::graph_helpers::VectorAllocator<_ND, T, false>,
        typename _ED_ALLOCATOR = offbynull::graph::graph_helpers::VectorAllocator<_ED, T, false>,
        bool error_check = true
    >
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    class pairwise_global_alignment_graph {
    public:
        using N = std::pair<T, T>;
        using E = std::pair<N, N>;
        using ED = _ED;
        using ND = _ND;

    private:
        offbynull::grid_graph::grid_graph::grid_graph<ND, ED, T, _ND_ALLOCATOR, _ED_ALLOCATOR, error_check> g;

    public:
        const T down_node_cnt;
        const T right_node_cnt;

        pairwise_global_alignment_graph(
            T _down_node_cnt,
            T _right_node_cnt,
            ED indel_weight = {},
            _ND_ALLOCATOR nd_container_creator = {},
            _ED_ALLOCATOR ed_container_creator = {}
        )
        : g{_down_node_cnt, _right_node_cnt, indel_weight, nd_container_creator, ed_container_creator}
        , down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt} {}

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            g.update_node_data(node, std::forward<ND>(data));
        }

        ND& get_node_data(const N& node) {
            return g.get_node_data(node);
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            g.update_edge_data(edge, std::forward<ED>(data));
        }

        ED& get_edge_data(const E& edge) {
            return std::get<2>(get_edge(edge));
        }

        N get_edge_from(const E& edge) {
            return std::get<0>(get_edge(edge));
        }

        N get_edge_to(const E& edge) {
            return std::get<1>(get_edge(edge));
        }

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge(edge);
        }

        auto get_root_nodes() {
            return g.get_root_nodes();
        }

        N get_root_node() {
            return g.get_root_node();
        }

        auto get_leaf_nodes() {
            return g.get_leaf_nodes();
        }

        auto get_nodes() {
            return g.get_nodes();
        }

        auto get_edges() {
            return g.get_edges();
        }

        bool has_node(const N& node) {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) {
            return g.has_edge(edge);
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs_full(node);
        }

        std::tuple<E, N, N, ED&> get_output_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_output_full(node);
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs_full(node);
        }

        std::tuple<E, N, N, ED&> get_input_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_input_full(node);
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs(node);
        }

        E get_output(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_output();
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs(node);
        }

        E get_input(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_input(node);
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.has_outputs(node);
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.has_inputs(node);
        }

        size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_out_degree(node);
        }

        size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_in_degree(node);
        }

        template<typename ELEM>
        void assign_weights(
            const auto& v,  // random access container
            const auto& w,  // random access container
            std::function<ED(const std::optional<std::reference_wrapper<const ELEM>>&, const std::optional<std::reference_wrapper<const ELEM>>&)> weight_lookup
        ) {
            static_assert(std::is_same_v<ELEM, std::decay_t<decltype(*v.begin())>>, "ELEM is wrong");
            if constexpr (error_check) {
                if (down_node_cnt != v.size() + 1u || right_node_cnt != w.size() + 1u) {
                    throw std::runtime_error("Mismatching node count");
                }
            }
            for (const auto& edge : get_edges()) {
                const auto& [n1, n2] { edge };
                const auto& [n1_down, n1_right] { n1 };
                const auto& [n2_down, n2_right] { n2 };
                std::optional<std::reference_wrapper<const ELEM>> v_elem { std::nullopt };
                if (n1_down + 1u == n2_down) {
                    v_elem = { v[n1_down] };
                }
                std::optional<std::reference_wrapper<const ELEM>> w_elem { std::nullopt };
                if (n1_right + 1u == n2_right) {
                    w_elem = { w[n1_right] };
                }
                update_edge_data(edge, weight_lookup(v_elem, w_elem));
            }
        }

        template<typename ELEM>
        static std::optional<std::tuple<std::optional<std::reference_wrapper<const ELEM>>, std::optional<std::reference_wrapper<const ELEM>>>> edge_to_elements(
            const E& edge,
            const auto& v,  // random access container
            const auto& w   // random access container
        ) {
            const auto& [n1, n2] {edge};
            const auto& [n1_down, n1_right] {n1};
            const auto& [n2_down, n2_right] {n2};
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                if constexpr (error_check) {
                    if (n1_down >= v.size() or n1_right >= w.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return { { { v[n1_down] }, { w[n1_right] } } };
            } else if (n1_down + 1u == n2_down && n1_right == n2_right) {
                if constexpr (error_check) {
                    if (n1_down >= v.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return { { { v[n1_down] }, std::nullopt } };
            } else if (n1_down == n2_down && n1_right + 1u == n2_right) {
                if constexpr (error_check) {
                    if (n1_right >= w.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return { { std::nullopt, { v[n1_down] } } };
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
        }
    };
}
#endif //PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
