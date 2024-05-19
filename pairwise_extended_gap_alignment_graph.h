#ifndef PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
#define PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H

#include <ranges>
#include <tuple>
#include <stdexcept>
#include <format>
#include <utility>
#include <vector>
#include <functional>
#include "boost/container/static_vector.hpp"
#include "boost/container/small_vector.hpp"
#include "graph_helpers.h"
#include "utils.h"

namespace offbynull::pairwise_aligner::extended_gap {
    enum class layer : uint8_t {
        DIAGONAL,
        DOWN,
        RIGHT
    };

    template<typename _ND, typename _ED, typename T>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    struct slot {
        using ED = _ED;
        using ND = _ND;
        ND down_nd;
        ND diagonal_nd;
        ND right_nd;
        ED to_next_diagonal_ed;  // match edges between nodes in the diagonal layer
        // gap edges between nodes in down/right layers all use the same _ED obj, so not placed here
        // indel edges from diagonal layer to down/right layers all use the same _ED obj, so not placed here
        // freeride edges from down/right layers to diagonal layer all use the same _ED obj, so not placed here   
    };

    template<
        typename _ND,
        typename _ED,
        typename T = unsigned int,
        typename _SLOT_ALLOCATOR = offbynull::graph::graph_helpers::VectorAllocator<slot<_ND, _ED, T>, T, false>,
        bool error_check = true
    >
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    class pairwise_extended_alignment_graph {
    public:
        using N = std::tuple<layer, T, T>;
        using ND = _ND;
        using E = std::pair<N, N>;
        using ED = _ED;

    private:
        decltype(std::declval<_SLOT_ALLOCATOR>().allocate(0u, 0u)) slots;
        _ED extended_indel_ed;
        _ED initial_indel_ed;
        _ED freeride_ed;

        auto construct_full_edge(N n1, N n2) {
            return std::tuple<E, N, N, ED*> {
                E { n1, n2 },
                n1,
                n2,
                &this->get_edge_data(E { n1, n2 })
            };
        }

        size_t to_raw_idx(size_t n_down, size_t n_right) {
            return (n_down * right_node_cnt) + n_right;
        }

    public:
        const T down_node_cnt;
        const T right_node_cnt;

        pairwise_extended_alignment_graph(
            T _down_node_cnt,
            T _right_node_cnt,
            ED initial_indel_weight = 0.0,
            ED extended_indel_weight = 0.0,
            ED freeride_weight = 0.0,
            _SLOT_ALLOCATOR slot_container_creator = {}
        )
        : down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt}
        , slots{slot_container_creator.allocate(_down_node_cnt, _right_node_cnt)}
        , initial_indel_ed{initial_indel_weight}
        , extended_indel_ed{extended_indel_weight}
        , freeride_ed{freeride_weight} {}

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            if (node.type == layer::DIAGONAL) {
                this->slots[to_raw_idx(n_down, n_right)].diagonal_nd = std::forward<ND>(data);
            } else if (node.type == layer::DOWN) {
                this->slots[to_raw_idx(n_down, n_right)].down_nd = std::forward<ND>(data);
            } else if (node.type == layer::RIGHT) {
                this->slots[to_raw_idx(n_down, n_right)].right_nd = std::forward<ND>(data);
            }
        }

        ND& get_node_data(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            if (node.type == layer::DIAGONAL) {
                return this->slots[to_raw_idx(n_down, n_right)].diagonal_nd;
            } else if (node.type == layer::DOWN) {
                return this->slots[to_raw_idx(n_down, n_right)].down_nd;
            } else if (node.type == layer::RIGHT) {
                return this->slots[to_raw_idx(n_down, n_right)].right_nd;
            }
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            const auto& [n1_layer, n1_down, n1_right] = edge.first;
            auto [n2_layer, n2_down, n2_right] = edge.second;
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                this->slots[to_raw_idx(n1_down, n1_right)].to_next_diagonal_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DOWN) {  // gap
                extended_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                extended_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN) {  // indel
                initial_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT) {  // indel
                initial_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL) {  // freeride
                freeride_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL) {  // freeride
                freeride_ed = std::forward<ED>(data);
            }
        }

        ED& get_edge_data(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            const auto& [n1_layer, n1_down, n1_right] = edge.first;
            auto [n2_layer, n2_down, n2_right] = edge.second;
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                return this->slots[to_raw_idx(n1_down, n1_right)].to_next_diagonal_ed;
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DOWN) {  // gap
                return extended_indel_ed;
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                return extended_indel_ed;
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN) {  // indel
                return initial_indel_ed;
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT) {  // indel
                return initial_indel_ed;
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_ed;
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_ed;
            }
        }

        N get_edge_from(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.first;
        }

        N get_edge_to(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.second;
        }

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::tuple<N, N, ED&> {this->get_edge_from(edge), this->get_edge_from(edge), this->get_edge_data(edge)};
        }

        auto get_root_nodes() {
            return std::ranges::single_view { std::tuple<layer, T, T>(layer::DIAGONAL, 0u, 0u) };
        }

        N get_root_node() {
            return std::tuple<layer, T, T>(layer::DIAGONAL, 0u, 0u);
        }

        auto get_leaf_nodes() {
            return std::ranges::single_view { std::tuple<layer, T, T>(layer::DIAGONAL, down_node_cnt - 1u, right_node_cnt - 1u) };
        }

        auto get_nodes() {
            auto diagonal_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return std::tuple<layer, T, T>(layer::DIAGONAL, n_down, n_right);
                })
            };
            auto down_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(1u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return std::tuple<layer, T, T>(layer::DOWN, n_down, n_right);
                })
            };
            auto right_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(1u, right_node_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return std::tuple<layer, T, T>(layer::RIGHT, n_down, n_right);
                })
            };
            return concat_view(
                std::move(diagonal_layer_nodes),
                concat_view(
                    std::move(down_layer_nodes),
                    std::move(right_layer_nodes)
                )
            );
        }

        auto get_edges() {
            auto diagonal_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return this->get_outputs(std::tuple<layer, T, T> { layer::DIAGONAL, n_down, n_right });
                })
                | std::views::join
            };
            auto down_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(1u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return this->get_outputs(std::tuple<layer, T, T> { layer::DOWN, n_down, n_right });
                })
                | std::views::join
            };
            auto right_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(1u, right_node_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return this->get_outputs(std::tuple<layer, T, T> { layer::RIGHT, n_down, n_right });
                })
                | std::views::join
            };
            return concat_view(
                std::move(diagonal_layer_edges),
                concat_view(
                    std::move(down_layer_edges),
                    std::move(right_layer_edges)
                )
            );
        }

        bool has_node(const N& node) {
            const auto& [n_layer, n_down, n_right] { node };
            return (n_layer == layer::DIAGONAL && n_down < down_node_cnt && n_down >= 0u && n_right < right_node_cnt && n_right >= 0u)
                || (n_layer == layer::DOWN && n_down < down_node_cnt && n_down >= 1u && n_right < right_node_cnt && n_right >= 0u)
                || (n_layer == layer::RIGHT && n_down < down_node_cnt && n_down >= 0u && n_right < right_node_cnt && n_right >= 1u);
        }

        bool has_edge(const E& edge) {
            const auto& [n1_layer, n1_down, n1_right] { edge.first };
            const auto& [n2_layer, n2_down, n2_right] { edge.second };
            return (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL && n1_down + 1u == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // match
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN && n1_down + 1u == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // initial gap (down)
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT && n1_down == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt) // initial gap (right)
                || (n1_layer == layer::DOWN && n2_layer == layer::DOWN && n1_down > 0u && n1_down + 1u == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // extended gap (down)
                || (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT && n1_right > 0u && n1_down == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt) // extended gap (right)
                || (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL && n1_down > 0u && n1_down == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // freeride (down)
                || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL && n1_right > 0u && n1_down == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt); // freeride (right)
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            boost::container::static_vector<std::tuple<E, N, N, ED*>, 3> ret {};
            const auto& [n1_layer, n1_down, n1_right] { node };
            if (n1_layer == layer::DIAGONAL) {
                if (n1_down == down_node_cnt - 1u && n1_right == right_node_cnt - 1u) {
                    // do nothing
                } else if (n1_down < down_node_cnt - 1u && n1_right < right_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_down + 1u, n1_right + 1u }));
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_down + 1u, n1_right }));
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_down, n1_right + 1u }));
                } else if (n1_right == right_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_down + 1u, n1_right }));
                } else if (n1_down == down_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_down, n1_right + 1u }));
                }
            } else if (n1_layer == layer::DOWN) {
                ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_down, n1_right }));
                if (n1_down < down_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_down + 1u, n1_right }));
                }
            } else if (n1_layer == layer::RIGHT) {
                ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_down, n1_right }));
                if (n1_right < right_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_down, n1_right + 1u }));
                }
            }
            return std::move(ret)
                | std::views::transform([](const auto& edge_full) noexcept {
                    const auto& [e, n1, n2, ed_ptr] = edge_full;
                    return std::tuple<E, N, N, ED&>(e, n1, n2, *ed_ptr);
                });
        }

        std::tuple<E, N, N, ED&> get_output_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_outputs_full(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            boost::container::static_vector<std::tuple<E, N, N, ED*>, 3> ret {};
            const auto& [n2_layer, n2_down, n2_right] { node };
            if (n2_layer == layer::DIAGONAL) {
                if (n2_down == 0u && n2_right == 0u) {
                    // do nothing
                } else if (n2_down > 0u && n2_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_down - 1u, n2_right - 1u }, node));
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_down, n2_right }, node));
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_down, n2_right }, node));
                } else if (n2_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_down, n2_right }, node));
                } else if (n2_down > 0u) {
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_down, n2_right }, node));
                }
            } else if (n2_layer == layer::DOWN) {
                if (n2_down > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_down - 1u, n2_right }, node));
                }
                if (n2_down > 1u) {
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_down - 1u, n2_right }, node));
                }
            } else if (n2_layer == layer::RIGHT) {
                if (n2_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_down, n2_right - 1u }, node));
                }
                if (n2_right > 1u) {
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_down, n2_right - 1u }, node));
                }
            }
            return std::move(ret)
                | std::views::transform([](const auto& edge_full) noexcept {
                    const auto& [e, n1, n2, ed_ptr] = edge_full;
                    return std::tuple<E, N, N, ED&>(e, n1, n2, *ed_ptr);
                });
        }

        std::tuple<const E&, const N&, const N&, ED&> get_input_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_inputs_full(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_inputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        E get_output(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_outputs(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        E get_input(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_inputs(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size() > 0u;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size() > 0u;
        }

        size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size();
        }

        size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size();
        }

        template<typename ELEM>
        void assign_weights(
            const auto& v,  // random access container
            const auto& w,  // random access container
            std::function<ED(const std::optional<std::reference_wrapper<const ELEM>>&, const std::optional<std::reference_wrapper<const ELEM>>&)> && weight_lookup,
            const ED gap_weight,
            const ED freeride_weight = {}
        ) {
            static_assert(std::is_same_v<ELEM, std::decay_t<decltype(*v.begin())>>, "ELEM is wrong");
            if constexpr (error_check) {
                if (down_node_cnt != v.size() + 1u || right_node_cnt != w.size() + 1u) {
                    throw std::runtime_error("Mismatching node count");
                }
            }
            for (const auto& edge : get_edges()) {
                const auto& [n1, n2] { edge };
                const auto& [n1_layer, n1_down, n1_right] { n1 };
                const auto& [n2_layer, n2_down, n2_right] { n2 };
                if ((n1_layer == layer::DOWN && n2_layer == layer::DOWN)
                        || n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                    update_edge_data(edge, gap_weight);
                } else if ((n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL)
                        || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL)) {  // freeride
                    update_edge_data(edge, 0.0);
                } else {
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
        }

        template<typename ELEM>
        static std::optional<std::tuple<std::optional<std::reference_wrapper<const ELEM>>, std::optional<std::reference_wrapper<const ELEM>>>> edge_to_elements(
            const E& edge,
            const auto& v,  // random access container
            const auto& w   // random access container
        ) {
            const auto& [n1, n2] { edge };
            const auto& [n1_layer, n1_down, n1_right] { n1 };
            const auto& [n2_layer, n2_down, n2_right] { n2 };
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                    if constexpr (error_check) {
                        if (n1_down >= v.size() or n1_right >= w.size()) {
                            throw std::runtime_error("Out of bounds");
                        }
                    }
                    return { { { v[n1_down] }, { w[n1_right] } } };
                }
            } else if ((n1_layer == layer::DOWN && n2_layer == layer::DOWN)  // extended indel
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN)) {  // indel
                if (n1_down + 1u == n2_down && n1_right == n2_right) {
                    if constexpr (error_check) {
                        if (n1_down >= v.size()) {
                            throw std::runtime_error("Out of bounds");
                        }
                    }
                    return { { { v[n1_down] }, std::nullopt } };
                }
            } else if ((n1_layer == layer::RIGHT && n2_layer == layer::RIGHT)  // extended indel
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT)) {  // indel
                if (n1_down == n2_down && n1_right + 1u == n2_right) {
                    if constexpr (error_check) {
                        if (n1_right >= w.size()) {
                            throw std::runtime_error("Out of bounds");
                        }
                    }
                    return { { std::nullopt, { v[n1_down] } } };
                }
            } else if ((n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL)  // freeride
                || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL)) {  // freeride
                return std::nullopt;
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
        }
    };
}
#endif //PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
