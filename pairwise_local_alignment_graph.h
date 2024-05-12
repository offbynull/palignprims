#ifndef PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
#define PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H

#include <ranges>
#include <tuple>
#include <stdexcept>
#include <format>
#include <utility>
#include <vector>
#include <functional>
#include "grid_graph_4.h"
#include "utils.h"

namespace offbynull::pairwise_aligner::local {
    enum class edge_type : uint8_t {
        FREE_RIDE,
        NORMAL
    };

    template<typename T = unsigned int>
    class edge {
    public:
        using N = std::pair<T, T>;
        edge_type type;
        std::pair<N, N> inner_edge;

        std::strong_ordering operator<=>(const edge& rhs) const = default;
    };

    template<typename _ED, typename T = unsigned int>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    class node_data {
    private:
        using N = std::pair<T, T>;
        using E = edge<T>;
        using ED = _ED;
    public:
        E backtracking_edge;
        ED backtracking_weight;
    };

    template<typename _ED, typename _ND_CONTAINER, typename _ED_CONTAINER, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    class pairwise_local_alignment_graph {
    public:
        using N = std::pair<T, T>;
        using E = edge<T>;
        using ED = _ED;
        using ND = node_data<_ED, T>;

    private:
        offbynull::grid_graph::grid_graph::grid_graph<ND, _ED, _ND_CONTAINER, _ED_CONTAINER, T, error_check> g;
        ED freeride_ed;

    public:
        pairwise_local_alignment_graph(
            T _down_node_cnt,
            T _right_node_cnt,
            std::function<_ND_CONTAINER(T, T)> nd_container_creator,
            std::function<_ED_CONTAINER(T, T)> ed_container_creator
        )
        : g{_down_node_cnt, _right_node_cnt, nd_container_creator, ed_container_creator}, freeride_ed{0.0} {}

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
            if (edge.type == edge_type::FREE_RIDE) {
                freeride_ed = std::forward<ED>(data);
            } else {
                g.update_edge_data(edge.inner_edge, data);
            }
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
            if (edge.type == edge_type::FREE_RIDE) {
                auto [n1, n2] = edge.inner_edge;
                return std::tuple<N, N, ED&> {n1, n2, freeride_ed};
            } else {
                return g.get_edge(edge.inner_edge);
            }
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
            auto from_src_range {
                std::views::cartesian_product(
                    std::views::iota(0u, g.down_node_cnt),
                    std::views::iota(0u, g.right_node_cnt)
                )
                | std::views::drop(1) // drop 0,0
                | std::views::transform([&](const auto & p) noexcept {
                    N n1 { 0u, 0u };
                    N n2 { std::get<0>(p), std::get<1>(p) };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto to_sink_range {
                std::views::cartesian_product(
                    std::views::iota(0u, g.down_node_cnt),
                    std::views::iota(0u, g.right_node_cnt)
                )
                | ( std::views::reverse | std::views::drop(1) | std::views::reverse ) // drop bottom right
                | std::views::transform([&](const auto & p) noexcept {
                    N n1 { std::get<0>(p), std::get<1>(p) };
                    N n2 { g.down_node_cnt - 1u, g.right_node_cnt - 1u };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto real_range {
                g.get_edges()
                | std::views::transform([&](const auto & p) noexcept {
                    return E { edge_type::NORMAL, p };
                })
            };
            // This should be using std::views::conat, but it wasn't included in this version of the C++ standard
            // library. The concat implementation below lacks several features (e.g. doesn't support the pipe operator)
            // and forcefully returns copies (concat_view::iterator::value_type ==
            // concat_view::iterator::reference_type).
            return concat_view {
                std::move(real_range),
                concat_view { from_src_range, to_sink_range }
            };
        }

        bool has_node(const N& node) {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) {
            if (edge.type == edge_type::NORMAL) {
                return g.has_edge(edge.inner_edge);
            } else {
                const auto & [n1, n2] { edge.inner_edge };
                const auto & [n1_down, n1_right] { n1 };
                const auto & [n2_down, n2_right] { n2 };
                if (n1_down == 0u && n1_right == 0u) {
                    if (n2_down == 0u && n2_right == 0u) {
                        return false;
                    } else if (n2_down <= g.down_node_cnt - 1u && n2_right <= g.right_node_cnt - 1u) {
                        return true;
                    } else {
                        return false;
                    }
                } else if (n2_down == g.down_node_cnt - 1u && n2_right == g.right_node_cnt - 1u) {
                    if (n1_down == g.down_node_cnt - 1u && n1_right == g.right_node_cnt - 1u) {
                        return false;
                    } else if (n1_down <= g.down_node_cnt - 1u && n1_right <= g.right_node_cnt - 1u) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }

        auto get_outputs_full(const N& node) {
            // auto normals {
            //     g.get_outputs(node)
            //     | std::views::transform([](const auto& e) noexcept { return E { edge_type::NORMAL, e }; })
            // };
            // boost::container::static_vector<E, 1> freerides;
            // if (node == N{ g.down_node_cnt - 1u, g.right_node_cnt - 1u }) {
            //     // do nothing
            // } else {
            //     freerides.push_back({ edge_type::FREE_RIDE, { node, { g.down_node_cnt - 1u, g.right_node_cnt - 1u } } });
            // }
            // auto freerides_no_ref {
            //     std::move(freerides)
            //     | std::views::transform([](const auto& e) noexcept -> E { return e; })
            // };
            // return concat_view(
            //     std::move(normals),
            //     std::move(freerides_no_ref)
            // );
            // // COMMENTED OUT BECAUSE concat_view DOESN'T SUPPORT PIPE OPERATOR, WHICH CALLERS USE.
            boost::container::static_vector<std::tuple<E, N, N, ED&>, 4> ret {};
            for (const auto& [e, n1, n2, ed_ptr] : g.get_outputs_full(node)) { // will iterate at-most 3 times
                E new_e { edge_type::NORMAL, e };
                ret.push_back(std::tuple<E, N, N, ED&> {new_e, n1, n2, ed_ptr});
            }
            // I had to use the for-loop above because g.get_outputs_full(node) doesn't allow pipe operator?
            const auto & [n_down, n_right] { node };
            if (!(n_down == g.down_node_cnt - 1u && n_right == g.right_node_cnt - 1u)) {
                ret.push_back(
                    {
                        { edge_type::FREE_RIDE, { node, { g.down_node_cnt - 1u, g.right_node_cnt - 1u } } },
                        node,
                        {g.down_node_cnt - 1u, g.right_node_cnt - 1u},
                        freeride_ed
                    }
                );
            }
            return ret;
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
            boost::container::static_vector<std::tuple<E, N, N, ED&>, 4> ret {};
            for (const auto& [e, n1, n2, ed_ptr] : g.get_inputs_full(node)) { // will iterate at-most 3 times
                E new_e { edge_type::NORMAL, e };
                ret.push_back(std::tuple<E, N, N, ED&> {new_e, n1, n2, ed_ptr});
            }
            // I had to use the for-loop above because g.get_outputs_full(node) doesn't allow pipe operator?
            const auto & [n_down, n_right] { node };
            if (!(n_down == 0u && n_right == 0u)) {
                ret.push_back(
                    {
                        { edge_type::FREE_RIDE, { { 0u, 0u }, node } },
                        { 0u, 0u },
                        node,
                        freeride_ed
                    }
                );
            }
            return ret;
        }

        std::tuple<E, N, N, ED&> get_input_full(const N& node) {
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
            return get_outputs_full(node) | std::views::transform([this](auto v) noexcept -> E { return std::get<0>(v); });
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
            return this->get_inputs_full(node) | std::views::transform([this](auto v) noexcept -> E { return std::get<0>(v); });
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
            return !this->get_outputs(node).empty();
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return !this->get_inputs(node).empty();
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
    };

    template<typename _ED, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_vector_grid(T down_cnt, T right_cnt) {
        size_t size = down_cnt * right_cnt;
        using ND = node_data<_ED, T>;
        return pairwise_local_alignment_graph<_ED, std::vector<ND>, std::vector<offbynull::grid_graph::grid_graph::edge_data_set<_ED>>, T, error_check> {
            down_cnt,
            right_cnt,
            [size](T, T) { return std::vector<ND>(size); },
            [size](T, T) { return std::vector<offbynull::grid_graph::grid_graph::edge_data_set<_ED>>(size); }
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_array_grid() {
        constexpr size_t size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
        using ND = node_data<_ED, T>;
        return pairwise_local_alignment_graph<_ED, std::array<ND, size>, std::array<offbynull::grid_graph::grid_graph::edge_data_set<_ED>, size>, T, error_check> {
            STATIC_DOWN_CNT,
            STATIC_RIGHT_CNT,
            [](T, T) { return std::array<ND, size>{}; },
            [](T, T) { return std::array<offbynull::grid_graph::grid_graph::edge_data_set<_ED>, size> {}; }
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_small_vector_grid(T down_cnt, T right_cnt) {
        constexpr size_t stack_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
        size_t actual_size = down_cnt * right_cnt;
        using ND = node_data<_ED, T>;
        return pairwise_local_alignment_graph<_ED, boost::container::small_vector<ND, stack_size>, boost::container::small_vector<offbynull::grid_graph::grid_graph::edge_data_set<_ED>, stack_size>, T, error_check> {
            down_cnt,
            right_cnt,
            [actual_size](T, T) { return boost::container::small_vector<ND, stack_size>(actual_size); },
            [actual_size](T, T) { return boost::container::small_vector<offbynull::grid_graph::grid_graph::edge_data_set<_ED>, stack_size>(actual_size); }
        };
    }

    template<typename _ED, size_t STATIC_DOWN_CNT, size_t STATIC_RIGHT_CNT, typename T = unsigned int, bool error_check = true>
        requires std::is_floating_point_v<_ED> && std::is_integral_v<T> && std::is_unsigned_v<T>
    auto create_static_vector_grid(T down_cnt, T right_cnt) {
        constexpr size_t stack_size = STATIC_DOWN_CNT * STATIC_RIGHT_CNT;
        size_t actual_size = down_cnt * right_cnt;
        using ND = node_data<_ED, T>;
        return pairwise_local_alignment_graph<_ED, boost::container::static_vector<ND, stack_size>, boost::container::static_vector<offbynull::grid_graph::grid_graph::edge_data_set<_ED>, stack_size>, T, error_check> {
            down_cnt,
            right_cnt,
            [actual_size](T, T) { return boost::container::static_vector<ND, stack_size>(actual_size); },
            [actual_size](T, T) { return boost::container::static_vector<offbynull::grid_graph::grid_graph::edge_data_set<_ED>, stack_size>(actual_size); }
        };
    }
}
#endif //PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
